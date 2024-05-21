#include "TcpServer.h"
#include "EchoServer.h"
#include <signal.h>


EchoServer *echoserver;

// 信号处理函数，功能是停止服务程序
void Stop(int sig)
{
    printf("sig=%d\n", sig);
    //调用EchoServer::Stop()停止服务程序
    echoserver->Stop();
    delete echoserver;
    exit(0);
}

int main(int argc,char *argv[])
{
    if (argc != 3) 
    { 
        printf("usage: ./tcpepoll ip port\n"); 
        printf("example: ./tcpepoll 192.168.71.132 5085\n\n"); 
        return -1; 
    }

    signal(SIGINT, Stop);
    signal(SIGTERM, Stop);
    echoserver = new EchoServer(argv[1], atoi(argv[2]), 3, 8);
    echoserver->Start();

    return 0;
}