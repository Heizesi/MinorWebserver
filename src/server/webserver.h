#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "../epoller/epoller.h"
#include "../log/log.h"
#include "../http/httpconn.h"
#include "../threadpool/threadpool.h"
#include "../timerheap/timerheap.h"
#include <cstdint>
#include <memory>
#include <netinet/in.h>
#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

class WebServer
{
private:
    std::unique_ptr<TimerHeap> timer;
    std::unique_ptr<ThreadPool> threadpool;
    std::unique_ptr<Epoller> epoller;
    std::unordered_map<int, HttpConn> users;
    uint32_t listenEvent;
    uint32_t connEvent;
    int port;
    bool openLinger;
    int timeoutMS;
    bool isClose;
    int listenFd;
    char* srcDir;
    static const int max_fd=65536;
    
    static int setFdNonblock(int fd);
    bool initScoket();
    void initEventMode();
    void addClient(int fd,sockaddr_in addr);
    void doListen();
    void doWrite(HttpConn* client);
    void doRead(HttpConn* client);
    void sendError(int fd,const char* info);
    void incTimeout(HttpConn* client);
    void closeConn(HttpConn* client);
    void onRead(HttpConn* client);
    void onWrite(HttpConn* client);
    void onProcess(HttpConn* client);
public:
    WebServer(int port,int timeoutMS,bool optLinger,int threadNum,bool openLog,int logLevel,int logQueSize);
    ~WebServer();
    void start();
};
#endif //WEBSERVER_H