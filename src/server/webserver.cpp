#include "webserver.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/socket.h>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <netinet/in.h>
#include <ostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

WebServer::WebServer(int port,int timeoutMS,bool optLinger,int threadNum,bool openLog,int logLevel,int logQueSize):port(port),openLinger(optLinger),timeoutMS(timeoutMS),isClose(false),timer(new TimerHeap()),threadpool(new ThreadPool(threadNum)),epoller(new Epoller())
{
    std::cout<<"init server"<<std::endl;
    srcDir=getcwd(nullptr, 256);
    std::strncat(srcDir,"/resources/",16);
    HttpConn::userCount=0;
    HttpConn::srcDir=srcDir;
    initEventMode();
    std::cout<<"init mode"<<std::endl;
    if(openLog)
    {
        Log::Instance()->init(logLevel,"./log",".log",logQueSize);
        if(isClose)
        {
            LOG_ERROR("Server init error!");
        }
        else
        {
            LOG_INFO("Server init");
        }
    }
    if(!initScoket())
    {
        isClose=true;
    }
    std::cout<<"init socket"<<std::endl;
}

WebServer::~WebServer()
{
    close(listenFd);
    isClose=true;
    free(srcDir);
}

int WebServer::setFdNonblock(int fd)
{
    return fcntl(fd, F_SETFL,fcntl(fd, F_GETFD,0)|O_NONBLOCK);
}

void WebServer::initEventMode()
{
    listenEvent=EPOLLRDHUP|EPOLLET;
    connEvent=EPOLLONESHOT|EPOLLRDHUP|EPOLLET;
    return ;
}

bool WebServer::initScoket()
{
    int ret=-1;
    struct sockaddr_in addr;
    if(port>65535 || port<1024)
    {
        LOG_ERROR("Port:%d error!",port);
        return false;
    }
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    addr.sin_port=htons(port);
    struct linger optLinger={0};
    if(openLinger)
    {
        optLinger.l_onoff=1;
        optLinger.l_linger=1;
    }
    listenFd=socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd<0)
    {
        LOG_ERROR("Port:%d create socket error!",port);
        return false;
    }
    ret=setsockopt(listenFd, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if(ret<0)
    {
        close(listenFd);
        LOG_ERROR("Port:%d init linger error!",port);
        return false;
    }
    int optval=1;
    ret=setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret==-1)
    {
        LOG_ERROR("setsockpot error!");
        close(listenFd);
        return false;
    }
    ret=bind(listenFd, (struct sockaddr*)&addr,sizeof(addr));
    if(ret<0)
    {
        LOG_ERROR("Bind port:%d error!",port);
        close(listenFd);
        return false;
    }
    ret=listen(listenFd, 6);
    if(ret<0)
    {
        LOG_ERROR("Listen port:%d error!",port);
        close(listenFd);
        return false;
    }
    ret=epoller->addFd(listenFd, listenEvent|EPOLLIN);
    if(ret==0)
    {
        LOG_ERROR("Add listen epoll error!");
        close(listenFd);
        return false;
    }
    setFdNonblock(listenFd);
    LOG_INFO("server port:%d",port);
    return true;
}

void WebServer::sendError(int fd,const char* info)
{
    int ret=send(fd,info,std::strlen(info),0);
    if(ret<0)
    {
        LOG_WARN("send error to client:%d error!",fd);
    }
    close(fd);
}

void WebServer::closeConn(HttpConn* client)
{
    LOG_INFO("client:%d close",client->getFd());
    epoller->delFd(client->getFd());
    client->closeConn();
    return ;
}

void WebServer::addClient(int fd,sockaddr_in addr)
{
    users[fd].init(fd, addr);
    if(timeoutMS>0)
    {
        timer->addNode(fd, timeoutMS, std::bind(&WebServer::closeConn,this,&users[fd]));
    }
    epoller->addFd(fd, EPOLLIN|connEvent);
    setFdNonblock(fd);
    LOG_INFO("Client:%d connected",users[fd].getFd());
    return ;
}

void WebServer::doListen()
{
    struct sockaddr_in addr;
    socklen_t len=sizeof(addr);
    while(true)
    {
        int fd=accept(listenFd, (struct sockaddr*)&addr, &len);
        if(fd<=0)
        {
            return ;
        }
        else if(HttpConn::userCount>=max_fd)
        {
            sendError(fd, "Server busy!");
            LOG_WARN("client is too more");
            return ;
        }
        addClient(fd, addr);
    }
    return ;
}

void WebServer::doRead(HttpConn* client)
{
    incTimeout(client);
    threadpool->addTask(std::bind(&WebServer::onRead,this,client));
    return ;
}

void WebServer::doWrite(HttpConn* client)
{
    incTimeout(client);
    threadpool->addTask(std::bind(&WebServer::onWrite,this,client));
    return ;
}

void WebServer::incTimeout(HttpConn* client)
{
    if(timeoutMS>0)
    {
        timer->changePeriod(client->getFd(), timeoutMS);
    }
    return ;
}

void WebServer::onRead(HttpConn* client)
{
    int ret=-1;
    int readErrno=0;
    ret=client->read(&readErrno);
    if(ret<=0 && readErrno!=EAGAIN)
    {
        closeConn(client);
        return ;
    }
    onProcess(client);
    return ;
}

void WebServer::onWrite(HttpConn* client)
{
    int ret=-1;
    int saveErrno=0;
    ret=client->write(&saveErrno);
    if(client->leftWriteBytes()==0)
    {
        if(client->isKeepAlive())
        {
            onProcess(client);
            return ;
        }
    }
    else if(ret<0)
    {
        if(saveErrno==EAGAIN)
        {
            epoller->modFd(client->getFd(), connEvent|EPOLLOUT);
            return ;
        }
    }
    closeConn(client);
    return ;
}

void WebServer::onProcess(HttpConn* client)
{
    if(client->process())
    {
        epoller->modFd(client->getFd(), connEvent|EPOLLOUT);
    }
    else
    {
        epoller->modFd(client->getFd(), connEvent|EPOLLIN);
    }
    return ;
}

void WebServer::start()
{
    std::cout<<"server start"<<std::endl;
    int timeMS=-1;
    if(!isClose)
    {
        LOG_INFO("server start!");
    }
    while(!isClose)
    {
        if(timeoutMS>0)
        {
            timeMS=timer->getNextTick();
        }
        int eventCnt=epoller->wait(timeMS);
        for(int i=0;i<eventCnt;i++)
        {
            int fd=epoller->getEventFd(i);
            uint32_t events=epoller->getEvents(i);
            if(fd==listenFd)
            {
                doListen();
            }
            else if(events&(EPOLLRDHUP|EPOLLHUP|EPOLLERR))
            {
                closeConn(&users[fd]);
            }
            else if(events&EPOLLIN)
            {
                doRead(&users[fd]);
            }
            else if(events&EPOLLOUT)
            {
                doWrite(&users[fd]);
            }
            else
            {
                LOG_ERROR("unexpect event");
            }
        }
    }
    return ;
}