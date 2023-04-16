#include "server/webserver.h"
using namespace std;

int main()
{
    WebServer server(1316,60000,false,6,true,1,1024);
    server.start();
    return 0;
}