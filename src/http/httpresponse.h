#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <cstddef>
#include <string>
#include <unordered_map>
#include <sys/stat.h>
#include "../buffer/buffer.h"

class HttpResponse
{
private:
    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> CODE_PATH;

    char* mmFile;
    struct stat mmFileStat;

    int code;
    bool isKeepAlive;
    std::string path;
    std::string srcDir;

    void addState();
    void addHeader();
    void addContent();
    void errorRespon();
    std::string getFileType();
public:
    Buffer buffer;
    
    HttpResponse();
    ~HttpResponse();

    void init(const std::string& srcDir,std::string& path,bool isKeepAlive=0,int code=-1);
    void makeRespon();
    void unmapFile();
    char* File();
    size_t fileLen() const;
    void errorContent(std::string message);
    int getCode() const;
};
#endif // HTTP_RESPONSE_H