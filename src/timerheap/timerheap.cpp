#include "timerheap.h"
#include <chrono>
#include <cstddef>
#include <tuple>

void TimerHeap::swapNode(size_t i,size_t j)
{
    std::swap(heap[i],heap[j]);
    map[heap[i].id]=i;
    map[heap[j].id]=j;
    return ;
}

void TimerHeap::shiftUp(size_t idx)
{
    size_t father=(idx-1)/2;
    while(father>=0)
    {
        if(heap[father]<heap[idx])
        {
            break;
        }
        swapNode(father,idx);
        idx=father;
        father=(father-1)/2;
    }
    return ;
}

void TimerHeap::shiftDown(size_t idx)
{
    size_t num=heap.size();
    size_t leftson=idx*2+1;
    size_t maxson;
    while(leftson<num)
    {
        maxson=leftson;
        if(maxson+1<num)
        {
            if(heap[maxson]<heap[maxson+1])
            {
                maxson+=1;
            }
        }
        if(heap[idx]<heap[maxson])
        {
            break;
        }
        swapNode(idx,maxson);
        idx=maxson;
        leftson=maxson*2+1;
    }
    return ;
}

void TimerHeap::clearAll()
{
    heap.clear();
    map.clear();
    return ;
}

void TimerHeap::deleteNode(size_t idx)
{
    size_t end=heap.size()-1;
    if(idx<=end)
    {
        swapNode(idx,end);
        map.erase(heap[end].id);
        heap.pop_back();
        shiftDown(idx);
    }
    return ;
}

void TimerHeap::changePeriod(int id, int newPeriod)
{
    heap[map[id]].period=HRClock::now()+ms(newPeriod);
    shiftDown(map[id]);
}

void TimerHeap::addNode(int id, int timeout, const std::function<void ()> &fun)
{
    size_t idx;
    if(map.count(id)==0)
    {
        idx=heap.size();
        map[id]=idx;
        heap.push_back((Timer){id,HRClock::now()+ms(timeout),fun});
        shiftUp(idx);
    }
    else
    {
        changePeriod(id,timeout);
    }
    return ;
}

void TimerHeap::pop()
{
    deleteNode(0);
    return ;
}

void  TimerHeap::nextTick()
{
    if(heap.empty())
    {
        return ;
    }
    while(!heap.empty())
    {
        Timer timer=heap.front();
        if(std::chrono::duration_cast<ms>(timer.period-HRClock::now()).count()>0)
        {
            break;
        }
        timer.callBackFun();
        pop();
    }
    return ;
}

void TimerHeap::runFun(int id)
{
    if(heap.empty() || map.count(id)==0)
    {
        return ;
    }
    size_t idx=map[id];
    Timer timer=heap[idx];
    timer.callBackFun();
    deleteNode(idx);
    return ;
}

int TimerHeap::getNextTick()
{
    nextTick();
    size_t left=-1;
    if(!heap.empty())
    {
        left=std::chrono::duration_cast<ms>(heap.front().period-HRClock::now()).count();
        if(left<0)
        {
            left=0;
        }
    }
    return left;
}