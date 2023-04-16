#ifndef EPOLLER_H
#define EPOLLER_H

#include <cstddef>
#include <cstdint>
#include <sys/epoll.h>
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>

class Epoller
{
private:
    int epollFd;
    std::vector<struct epoll_event> events;
public:
    explicit Epoller(int maxEvent=1024);
    ~Epoller();
    
    bool addFd(int fd,uint32_t events);
    bool modFd(int fd,uint32_t events);
    bool delFd(int fd);
    int wait(int timeoutMs=-1);
    int getEventFd(size_t i) const;
    uint32_t getEvents(size_t i) const;
};
#endif //EPOLLER_H