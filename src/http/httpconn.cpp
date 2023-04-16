#include "httpconn.h"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <sys/types.h>

const char* HttpConn::srcDir;
std::atomic_int HttpConn::userCount;

void HttpConn::init(int sockfd, const sockaddr_in &addr)
{
    userCount++;
    this->addr=addr;
    fd=sockfd;
    resp.buffer.clearAll();
    req.buffer.clearAll();
    isClose=false;
    return ;
}

void HttpConn::closeConn()
{
    resp.unmapFile();
    if(!isClose)
    {
        isClose=true;
        userCount--;
        close(fd);
    }
    return ;
}

ssize_t HttpConn::read(int *saveErrno)
{
    ssize_t len=0;
    while(true)
    {
        len=req.buffer.readFromFd(fd, saveErrno);
        if(len<=0)
        {
            break;
        }
    }
    return len;
}

ssize_t HttpConn::write(int *saveErrno)
{
    ssize_t len=0;
    do
    {
        len=writev(fd,iov,iovcnt);
        if(len<=0)
        {
            break;
        }
        if(iov[0].iov_len+iov[1].iov_len==0)
        {
            break;
        }
        else if(static_cast<size_t>(len)>iov[0].iov_len)
        {
            iov[1].iov_base=(uint8_t*)iov[1].iov_base+(len-iov[0].iov_len);
            iov[1].iov_len=iov[1].iov_len-(len-iov[0].iov_len);
            if(iov[0].iov_len)
            {
                resp.buffer.clearAll();
                iov[0].iov_len=0;
            }
        }
        else
        {
            iov[0].iov_base=(uint8_t*)iov[0].iov_base+len;
            iov[0].iov_len=iov[0].iov_len-len;
            req.buffer.readPosInc(len);
        }
    }
    while(leftWriteBytes()>10240);
    return len;
}

bool HttpConn::process()
{
    req.init();
    if(req.buffer.readableSize()<=0)
    {
        return false;
    }
    else if(req.parse())
    {
        resp.init(srcDir, req.getPath(),req.isKeepAlive(),200);
    }
    else
    {
        resp.init(srcDir, req.getPath(),false,400);
    }
    resp.makeRespon();
    iov[0].iov_base=const_cast<char*>(resp.buffer.beginReadPos());
    iov[0].iov_len=resp.buffer.readableSize();
    iovcnt=1;
    if(resp.fileLen()>0 && resp.File())
    {
        iov[1].iov_base=resp.File();
        iov[1].iov_len=resp.fileLen();
        iovcnt=2;
    }
    return true;
}