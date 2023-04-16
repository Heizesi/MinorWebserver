#include "buffer.h"

Buffer::Buffer(int Buffersize) : buffer(Buffersize),readPtr(0),writePtr(0) {}

char* Buffer::beginPos()
{
    return &*buffer.begin();
}

const char* Buffer::beginPos() const
{
    return &*buffer.begin();
}

void Buffer::writePosInc(size_t len)
{
    writePtr+=len;
    return ;
}

size_t Buffer::readableSize() const
{
    return writePtr-readPtr;
}

size_t Buffer::writableSize() const
{
    return buffer.size()-writePtr;
}

const char* Buffer::beginWritePosConst() const
{
    return beginPos()+writePtr;
}

char* Buffer::beginWritePos()
{
    return beginPos()+writePtr;
}

const char* Buffer::beginReadPos() const
{
    return beginPos()+readPtr;
}

void Buffer::readPosInc(size_t len)
{
    readPtr+=len;
    return ;
}

void Buffer::readPosIncUntil(const char* pos)
{
    readPosInc(pos-beginReadPos());
    return ;
}

void Buffer::clearAll()
{
    bzero(&buffer[0],buffer.size());
    readPtr=0;
    writePtr=0;
    return ;
}

std::string Buffer::readableToStr()
{
    std::string str(beginReadPos(),readableSize());
    clearAll();
    return str;
}

void Buffer::append(const std::string& str)
{
    append(str.data(),str.length());
    return ;
}

void Buffer::append(const char* str,size_t len)
{
    if(writableSize()<=len)
    {
        makeSpace(len);
    }
    std::copy(str,str+len,beginWritePos());
    writePosInc(len);
    return ;
}

void Buffer::append(const void* data,size_t len)
{
    append(static_cast<const char*> (data),len);
    return ;
}

ssize_t Buffer::readFromFd(int fd,int* Errno)
{
    char tempBuffer[65535];
    struct iovec iov[2];
    const size_t writable=writableSize();

    iov[0].iov_base=beginPos()+writePtr;
    iov[0].iov_len=writable;
    iov[1].iov_base=tempBuffer;
    iov[1].iov_len=sizeof(tempBuffer);

    const ssize_t len=readv(fd,iov,2);
    if(len<0)
    {
        *Errno=errno;
    }
    else if(static_cast<size_t>(len)<=writable)
    {
        writePosInc(len);
    }
    else
    {
        writePtr=buffer.size();
        append(tempBuffer,len-writable);
    }
    return len;
}

ssize_t Buffer::writeToFd(int fd,int* Errno)
{
    size_t readSize=readableSize();
    ssize_t len=write(fd,beginReadPos(),readSize);
    if(len<0)
    {
        *Errno=errno;
        return len;
    }
    readPosInc(len);
    return len;
}

void Buffer::makeSpace(size_t len)
{
    if(writableSize()+readPtr<=len)
    {
        buffer.resize(writePtr+len+1);
    }
    else
    {
        size_t readable=readableSize();
        std::copy(beginPos()+readPtr,beginWritePos(),beginPos());
        readPtr=0;
        writePtr=readable;
    }
    return ;
}