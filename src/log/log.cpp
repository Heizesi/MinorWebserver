#include "log.h"
#include "blockqueue.h"
#include <bits/types/time_t.h>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <ctime>
#include <memory>
#include <mutex>
#include <strings.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <thread>
#include <utility>

Log::Log()
{
    lineCount=0;
    writeThread=nullptr;
    blockqueue=nullptr;
    today=0;
    fp=nullptr;
}

Log::~Log()
{
    if(writeThread && writeThread->joinable())
    {
        while(!blockqueue->empty())
        {
            blockqueue->flush();
        }
        blockqueue->close();
        writeThread->join();
    }
    if(fp)
    {
        std::lock_guard<std::mutex> locker(writeMutex);
        flush();
        fclose(fp);
    }
}

Log* Log::Instance()
{
    static Log inst;
    return &inst;
}

void Log::flushLogThread()
{
    Log::Instance()->asyncWrite();
    return ;
}

void Log::flush()
{
    blockqueue->flush();
    fflush(fp);
    return ;
}

int Log::getLevel()
{
    std::lock_guard<std::mutex> locker(writeMutex);
    return level;
}

void Log::setLevel(int level)
{
    std::lock_guard<std::mutex> locker(writeMutex);
    this->level=level;
    return ;
}

void Log::asyncWrite()
{
    std::string str="";
    while(blockqueue->pop(str))
    {
        std::lock_guard<std::mutex> locker(writeMutex);
        fputs(str.c_str(),fp);
    }
    return ;
}

void Log::init(int level,const char* path,const char* suffix,int maxQueueSize)
{
    isOpened=true;
    this->level=level;
    if(!blockqueue)
    {
        std::unique_ptr<BlockQueue<std::string> > newBlockQueue(new BlockQueue<std::string>);
        blockqueue=std::move(newBlockQueue);
        std::unique_ptr<std::thread> newThread(new std::thread(flushLogThread));
        writeThread=std::move(newThread);
    }
    lineCount=0;
    time_t timer=time(nullptr);
    struct tm* sysTime=localtime(&timer);
    struct tm t=*sysTime;
    this->path=path;
    this->suffix=suffix;
    char fileName[LOG_NAME_LEN]={0};
    snprintf(fileName,LOG_NAME_LEN-1,"%s/%04d_%02d_%2d%s",path,t.tm_year+1900,t.tm_mon+1,t.tm_mday,suffix);
    today=t.tm_mday;
    {
        std::lock_guard<std::mutex> locker(writeMutex);
        buffer.clearAll();
        if(fp)
        {
            flush();
            fclose(fp);
        }
        fp=fopen(fileName,"a");
        if(fp==nullptr)
        {
            mkdir(path,0777);
            fp=fopen(fileName,"a");
        }
    }
    return ;
}

void Log::appendLogLevel(int level)
{
    switch(level)
    {
        case 0:
        {
            buffer.append("[debug]: ",9);
            break;
        }
        case 1:
        {
            buffer.append("[info] : ",9);
            break;
        }
        case 2:
        {
            buffer.append("[warn] : ",9);
            break;
        }
        case 3:
        {
            buffer.append("[error]: ",9);
            break;
        }
        default:
        {
            buffer.append("[info] : ",9);
            break;
        }
    }
    return ;
}

void Log::write(int level, const char *format, ...)
{
    struct timeval now={0,0};
    gettimeofday(&now,nullptr);
    time_t tSec=now.tv_sec;
    struct tm* sysTime=localtime(&tSec);
    struct tm t=*sysTime;
    va_list vaList;
    if(today!=t.tm_mday || (lineCount && (lineCount%MAX_LINES==0)))
    {
        std::unique_lock<std::mutex> locker(writeMutex);
        locker.unlock();
        char newFile[LOG_NAME_LEN];
        char tail[36]={0};
        snprintf(tail, 36, "%04d_%02d_%02d",t.tm_year+1900,t.tm_mon+1,t.tm_mday);
        if(today!=t.tm_mday)
        {
            snprintf(newFile, LOG_NAME_LEN-72, "%s/%s%s",path,tail,suffix);
            today=t.tm_mday;
            lineCount=0;
        }
        else
        {
            snprintf(newFile, LOG_NAME_LEN-72, "%s/%s-%d%s",path,tail,(lineCount/MAX_LINES),suffix);
        }
        locker.lock();
        flush();
        fclose(fp);
        fp=fopen(newFile,"a");
    }
    {
        std::unique_lock<std::mutex> locker(writeMutex);
        lineCount++;
        int n=snprintf(buffer.beginWritePos(), 128, "%04d-%02d-%02d %02d:%02d:%02d.%06ld",t.tm_year+1900,t.tm_mon+1,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec,now.tv_usec);
        buffer.writePosInc(n);
        appendLogLevel(level);
        va_start(vaList,format);
        int m=vsnprintf(buffer.beginWritePos(), buffer.writableSize(), format, vaList);
        va_end(vaList);
        buffer.writePosInc(m);
        buffer.append("\n\0",2);
        if(!blockqueue->full())
        {
            blockqueue->push_back(buffer.readableToStr());
        }
        else
        {
            fputs(buffer.beginReadPos(),fp);
        }
        buffer.clearAll();
    }
    return ;
}