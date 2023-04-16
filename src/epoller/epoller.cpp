#include "epoller.h"
#include <cstdint>
#include <sys/epoll.h>


Epoller::Epoller(int maxEvent):epollFd(epoll_create(512)),events(maxEvent){};

Epoller::~Epoller()
{
    close(epollFd);
}

bool Epoller::addFd(int fd, uint32_t events)
{
    if(fd<0)
    {
        return false;
    }
    epoll_event ev={0};
    ev.data.fd=fd;
    ev.events=events;
    if(epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev)==0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool Epoller::modFd(int fd, uint32_t events)
{
    if(fd<0)
    {
        return false;
    }
    epoll_event ev={0};
    ev.data.fd=fd;
    ev.events=events;
    if(epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev)==0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool  Epoller::delFd(int fd)
{
    if(fd<0)
    {
        return false;
    }
    epoll_event ev={0};
    if(epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, &ev)==0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int Epoller::wait(int timeoutMs)
{
    return epoll_wait(epollFd, &events[0], static_cast<int>(events.size()), timeoutMs);

}

int Epoller::getEventFd(size_t i) const
{
    return events[i].data.fd;
}

uint32_t Epoller::getEvents(size_t i) const
{
    return events[i].events;
}