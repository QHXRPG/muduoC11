all:client tcpepoll

client:client.cpp
	g++ -std=c++11 -g -o client client.cpp

tcpepoll:tcpepoll.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp EventLoop.cpp
	g++ -std=c++11 -g -o tcpepoll tcpepoll.cpp InetAddress.cpp Socket.cpp Epoll.cpp Channel.cpp EventLoop.cpp

clean:
	rm -f client tcpepoll
