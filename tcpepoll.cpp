/*
用于演示采用epoll模型实现网络通讯的服务端。
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>          
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/epoll.h>
#include <netinet/tcp.h>      // TCP_NODELAY需要包含这个头文件。
#include "InetAddress.h"
#include "Socket.h"
#include "Epoll.h"
#include "EventLoop.h"

int main(int argc,char *argv[])
{
    if (argc != 3) 
    { 
        printf("usage: ./tcpepoll ip port\n"); 
        printf("example: ./tcpepoll 192.168.71.131 5085\n\n"); 
        return -1; 
    }

    Socket servsock(createnonblocking());
    InetAddress servaddr(argv[1],atoi(argv[2]));             // 服务端的地址和协议。
    servsock.setreuseaddr(true);
    servsock.settcpnodelay(true);
    servsock.setreuseport(true);
    servsock.setkeepalive(true);
    servsock.bind(servaddr);
    servsock.listen();

    //创建事件循环的对象
    EventLoop loop;
    Channel *servchannel = new Channel(&loop, servsock.fd());  

    //将readcallback_绑定为servchannel实例对象中的Channel::newconnetion函数，并将servsock作为参数传给这个函数
    servchannel->setreadcallback(std::bind(&Channel::newconnetion, servchannel, &servsock));

    servchannel->enablereading();

    // 运行事件循环
    loop.run();

    return 0;
}