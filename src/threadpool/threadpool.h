#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>

class ThreadPool
{
private:
    struct Pool
    {
        std::mutex condMutex;
        std::condition_variable cond;
        std::queue<std::function<void()> > taskQueue;
        bool isClosed;
    };
    std::shared_ptr<Pool> pool;
public:
    ThreadPool(ThreadPool&&)=default;
    ThreadPool()=default;
    ~ThreadPool()
    {
        if(static_cast<bool>(pool))
        {
            {
                std::lock_guard<std::mutex> locker(pool->condMutex);
                pool->isClosed=true;
            }
            pool->cond.notify_all();
        }
    }
    ThreadPool(size_t poolSize=8): pool(std::make_shared<Pool>())
    {
        for(size_t i=0;i<poolSize;i++)
        {
            std::thread([pool=pool]
            {
                std::unique_lock<std::mutex> locker(pool->condMutex);
                while(true)
                {
                    if(!pool->taskQueue.empty())
                    {
                        auto task=std::move(pool->taskQueue.front());
                        pool->taskQueue.pop();
                        locker.unlock();
                        task();
                        locker.lock();
                    }
                    else if(pool->isClosed)
                    {
                        break;
                    }
                    else
                    {
                        pool->cond.wait(locker);
                    }
                }
            }).detach();
        }
    }

    template<class T>
    void addTask(T&& task)
    {
        {
            std::lock_guard<std::mutex> locker(pool->condMutex);
            pool->taskQueue.emplace(std::forward<T> (task));
        }
        pool->cond.notify_one();
    }
};
#endif //THREADPOOL_H