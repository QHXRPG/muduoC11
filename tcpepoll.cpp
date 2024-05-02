#include "TcpServer.h"
#include "EchoServer.h"

int main(int argc,char *argv[])
{
    if (argc != 3) 
    { 
        printf("usage: ./tcpepoll ip port\n"); 
        printf("example: ./tcpepoll 192.168.71.132 5085\n\n"); 
        return -1; 
    }

    EchoServer echoserver(argv[1], atoi(argv[2]));
    echoserver.Start();

    return 0;
}