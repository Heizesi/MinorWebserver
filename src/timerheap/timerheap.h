#ifndef TIMER_HEAP_H
#define HEAP_TIMER_H

#include <cstddef>
#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h> 
#include <functional> 
#include <assert.h> 
#include <chrono>
#include <vector>

typedef std::chrono::high_resolution_clock HRClock;
typedef std::chrono::milliseconds ms;

class TimerHeap
{
private:
    struct Timer
    {
        int id;
        HRClock::time_point period;
        std::function<void()> callBackFun;
        bool operator < (const Timer& t)
        {
            return period<t.period;
        }
    };
    
    std::vector<Timer> heap;
    std::unordered_map<int,size_t> map;

    void deleteNode(size_t idx);
    void shiftUp(size_t idx);
    void shiftDown(size_t idx);
    void swapNode(size_t i,size_t j);

public:
    TimerHeap()
    {
        heap.reserve(128);
    }

    void clearAll();

    ~TimerHeap()
    {
        clearAll();
    }

    void changePeriod(int id,int newPeriod);
    void addNode(int id,int timeout,const std::function<void()>& fun);
    void runFun(int id);
    void nextTick();
    void pop(); 
    int getNextTick();
};
#endif //TIMER_HEAP_H