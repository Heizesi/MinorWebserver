#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <cstddef>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <sys/time.h>

template<class T>
class BlockQueue
{
private:
    std::queue<T> queue;
    size_t maxSize;
    std::mutex condMutex;
    std::condition_variable consumerCond;
    std::condition_variable producerCond;
    bool isClosed;
public:
    explicit BlockQueue(size_t maxSize=1000): maxSize(maxSize)
    {
        isClosed=false;
    }
    ~BlockQueue()
    {
        close();
    }

    void close()
    {
        {
            std::lock_guard<std::mutex> locker(condMutex);
            while(!queue.empty())
            {
                queue.pop();
            }
            isClosed=true;
        }
        consumerCond.notify_all();
        producerCond.notify_all();
        return ;
    }

    void clear()
    {
        std::lock_guard<std::mutex> locker(condMutex);
        while(!queue.empty())
        {
            queue.pop();
        }
        return ;
    }

    bool empty()
    {
        std::lock_guard<std::mutex> locker(condMutex);
        return queue.empty();
    }

    bool full()
    {
        std::lock_guard<std::mutex> locker(condMutex);
        return queue.size()>=maxSize;
    }

    size_t size()
    {
        std::lock_guard<std::mutex> locker(condMutex);
        return (size_t)queue.size();
    }

    size_t getMaxSize()
    {
        std::lock_guard<std::mutex> locker(condMutex);
        return maxSize;
    }

    T front()
    {
        std::lock_guard<std::mutex> locker(condMutex);
        return queue.front();
    }

    T back()
    {
        std::lock_guard<std::mutex> locker(condMutex);
        return queue.back();
    }

    void push_back(const T& item)
    {
        std::unique_lock<std::mutex> locker(condMutex);
        while(size()>=getMaxSize())
        {
            producerCond.wait(locker);
        }
        queue.push(item);
        consumerCond.notify_one();
        return ;
    }

    bool pop(T& item)
    {
        std::unique_lock<std::mutex> locker(condMutex);
        while(!queue.empty())
        {
            consumerCond.wait(locker);
            if(isClosed)
            {
                return false;
            }
        }
        item=queue.front();
        queue.pop();
        producerCond.notify_one();
        return true;
    }

    void flush()
    {
        consumerCond.notify_one();
        return ;
    }
};
#endif // BLOCKQUEUE_H