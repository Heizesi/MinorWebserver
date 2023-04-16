#include "httprequest.h"
#include <algorithm>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>

const std::unordered_set<std::string> HttpRequest::DEFAULT_HTML
{
    "/index", "/register", "/login",
     "/welcome", "/video", "/picture"
};

void HttpRequest::init()
{
    method=path=version=body="";
    state=REQUEST_LINE;
    header.clear();
    post.clear();
    return ;
}

void HttpRequest::parsePath()
{
    if(path=="/")
    {
        path="/index.html"; 
    }
    else
    {
        for(auto &item: DEFAULT_HTML)
        {
            if(item==path)
            {
                path+=".html";
                break;
            }
        }
    }
    return ;
}

bool HttpRequest::parse()
{
    const char CRLF[]="\r\n";
    if(buffer.readableSize()<=0)
    {
        return false;
    }
    while(buffer.readableSize() && state!=FINISH)
    {
        const char* lineEnd=std::search(buffer.beginReadPos(),buffer.beginWritePosConst(),CRLF,CRLF+2);
        std::string line(buffer.beginReadPos(),lineEnd);
        switch(state)
        {
            case REQUEST_LINE:
            {
                if(!parseRequestLine(line))
                {
                    return false;
                }
                parsePath();
                break;
            }
            case HEADERS:
            {
                if(buffer.readableSize()<=2)
                {
                    state=FINISH;
                }
                break;
            }
            case BODY:
            {
                parseBody(line);
                break;
            }
            default:
            {
                break;
            }
        }
        if(lineEnd == buffer.beginWritePosConst())
        {
            break;
        }
        buffer.readPosIncUntil(lineEnd+2);
    }
    return true;
}

bool HttpRequest::isKeepAlive() const
{
    if(header.count("Connection") == 1)
    {
        return (header.find("Connection")->second == "keep-alive") && (version == "1.1");
    }
    return false;
}

bool HttpRequest::parseRequestLine(const std::string& line)
{
    std::regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch subMatch;
    if(std::regex_match(line,subMatch,patten))
    {
        method=subMatch[1];
        path=subMatch[2];
        version=subMatch[3];
        state=HEADERS;
        return true;
    }
    return false;
}

void HttpRequest::parseHeader(const std::string& line)
{
    std::regex patten("^([^:]*): ?(.*)$");
    std::smatch subMatch;
    if(std::regex_match(line,subMatch,patten))
    {
        header[subMatch[1]]=subMatch[2];
    }
    else
    {
        state=BODY;
    }
    return ;
}

void HttpRequest::parseBody(const std::string& line)
{
    body=line;
    parsePost();
    state=FINISH;
    return ;
}

void HttpRequest::parsePost()
{
    if(method=="POST" && header["Content-Type"]=="application/x-www-form-urlencoded")
    {
        parseFromUrlencoded();
    }
    return ;
}

void HttpRequest::parseFromUrlencoded()
{
    if(body.size()==0)
    {
        return ;
    }
    std::string key,value;
    int num=0;
    body.append("&");
    int n=body.size();
    int ptr1=0,ptr2=0;
    for(;ptr1<n;ptr1++)
    {
        char ch=body[ptr1];
        switch(ch)
        {
            case '=':
            {
                key=body.substr(ptr2,ptr1-ptr2);
                ptr2=ptr1+1;
                break;
            }
            case '+':
            {
                body[ptr1]=' ';
                break;
            }
            case '%':
            {
                num=converFromHex(body[ptr1+1]*16+converFromHex(body[ptr1+2]));
                body[ptr1+2]=num%10+'0';
                body[ptr1+1]=num/10+'0';
                ptr1+=2;
                break;
            }
            case '&':
            {
                value=body.substr(ptr2,ptr1-ptr2);
                ptr2=ptr1+1;
                post[key]=value;
                break;
            }
            default:
            {
                break;
            }
        }
    }
    return ;
}