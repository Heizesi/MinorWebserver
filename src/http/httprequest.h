#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <sstream>
#include <cstring>
#include <unordered_map>
#include <unordered_set>
#include <cstring>
#include <regex>
#include <errno.h>
#include "../buffer/buffer.h"
#include "../log/log.h"

class HttpRequest
{
private:
    static const std::unordered_set<std::string> DEFAULT_HTML;
    std::string method,path,version,body;
    std::unordered_map<std::string ,std::string> header;
    std::unordered_map<std::string, std::string> post;

    static int converFromHex(char ch)
    {
        if(ch >= 'A' && ch <= 'F') return ch -'A' + 10;
        if(ch >= 'a' && ch <= 'f') return ch -'a' + 10;
        return ch;
    }

    bool parseRequestLine(const std::string& line);
    void parseHeader(const std::string&line);
    void parseBody(const std::string& line);
    void parsePath();
    void parsePost();
    void parseFromUrlencoded();
public:
    enum PARSE_STATE
    {
        REQUEST_LINE=0,
        HEADERS,
        BODY,
        FINISH
    };
    enum HTTP_CODE
    {
        NO_REQUEST=0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURSE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };
    PARSE_STATE state;
    Buffer buffer;

    HttpRequest()
    {
        init();
    }
    ~HttpRequest()=default;

    void init();
    bool parse();
    bool isKeepAlive() const;

    std::string getPath() const
    {
        return path;
    }

    std::string& getPath()
    {
        return path;
    }

    std::string getMethod() const
    {
        return method;
    }

    std::string getVersion() const
    {
        return version;
    }

    std::string getPost(const std::string& key) const
    {
        if(post.count(key)==1)
        {
            return post.find(key)->second;
        }
        return "";
    }

    std::string getPost(const char* key) const
    {
        if(post.count(key)==1)
        {
            return post.find(key)->second;
        }
        return "";
    }
};
#endif //HTTP_REQUEST_H