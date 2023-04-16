#ifndef LOG_H
#define LOG_H

#include <sys/time.h>
#include "../buffer/buffer.h"
#include <bits/types/FILE.h>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include "../log/blockqueue.h"

class Log
{
private:
    Log();
    void appendLogLevel(int level);
    virtual ~Log();
    void asyncWrite();

    static const int LOG_PATH_LEN=256;
    static const int LOG_NAME_LEN=256;
    static const int MAX_LINES=100000;

    const char* path;
    const char* suffix;

    int maxLines;
    int lineCount;
    int today;
    bool isOpened;
    Buffer buffer;    
    int level;
    FILE* fp;
    std::unique_ptr<BlockQueue<std::string> > blockqueue;
    std::unique_ptr<std::thread> writeThread;
    std::mutex writeMutex;
public:
    void init(int level,const char* path="./log",const char* suffix=".log",int maxQueueSize=1000);
    
    static Log* Instance();
    static void flushLogThread();
    
    void write(int level,const char* format,...);
    void flush();
    int getLevel();
    void setLevel(int level);
    bool isOpen()
    {
        return isOpened;
    }
};

#define LOG_BASE(level,format,...)\
    do\
    {\
        Log* log=Log::Instance();\
        if(log->isOpen() && log->getLevel()<=level)\
        {\
            log->write(level,format,##__VA_ARGS__);\
            log->flush();\
        }\
    }while(0);

#define  LOG_DEBUG(format,...) do{LOG_BASE(0,format,##__VA_ARGS__)}while(0);
#define  LOG_INFO(format,...) do{LOG_BASE(1,format,##__VA_ARGS__)}while(0);
#define  LOG_WARN(format,...) do{LOG_BASE(2,format,##__VA_ARGS__)}while(0);
#define  LOG_ERROR(format,...) do{LOG_BASE(3,format,##__VA_ARGS__)}while(0);
#endif // LOG_H