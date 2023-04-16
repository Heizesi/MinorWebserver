#ifndef BUFFER_H
#define BUFFER_H

#include <cstring>   
#include <iostream>
#include <unistd.h>
#include <sys/uio.h>
#include <vector>
#include <atomic>

class Buffer
{
private:
    char* beginPos();
    const char* beginPos() const;
    void makeSpace(size_t len);
    
    std::vector<char> buffer;
    std::atomic<std::size_t> readPtr;
    std::atomic<std::size_t> writePtr;
public:
    Buffer(int Buffersize=1024);
    ~Buffer()=default;

    size_t readableSize() const;
    size_t writableSize() const;

    const char* beginReadPos() const;
    void clearAll();
    void writePosInc(size_t len);
    void readPosInc(size_t len);
    void readPosIncUntil(const char* pos);
    std::string readableToStr();

    char* beginWritePos();
    const char* beginWritePosConst() const;

    void append(const std::string& str);
    void append(const char* str,size_t len);
    void append(const void* data,size_t len);

    ssize_t readFromFd(int fd,int* Errno);
    ssize_t writeToFd(int fd,int* Errno);
};
#endif //BUFFER_H