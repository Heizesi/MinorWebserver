#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include "../http/httprequest.h"
#include "../http/httpresponse.h"
#include "../buffer/buffer.h"
#include <arpa/inet.h>
#include <atomic>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <cstdlib>

class HttpConn
{
private:
    int fd;
    struct sockaddr_in addr;
    bool isClose;
    int iovcnt;
    struct iovec iov[2];
    HttpResponse resp;
    HttpRequest req;
public:
    HttpConn()
    {
        fd=1;
        addr={0};
        isClose=true;
    }

    ~HttpConn()
    {
        closeConn();
    }

    static const char* srcDir;
    static std::atomic_int userCount;

    void init(int sockfd,const sockaddr_in& addr);
    ssize_t read(int* saveErrno);
    ssize_t write(int* saveErrno);
    void closeConn();
    bool process();
    
    int getFd() const
    {
        return fd;
    }
    
    int getPort() const
    {
        return addr.sin_port;
    }
    
    const char* getIP() const
    {
        return inet_ntoa(addr.sin_addr);
    }
    
    sockaddr_in getAddr() const
    {
        return addr;
    }
    
    int leftWriteBytes()
    {
        return iov[0].iov_len+iov[1].iov_len;
    }

    bool isKeepAlive() const
    {
        return req.isKeepAlive();
    }
};
#endif //HTTP_CONN_H