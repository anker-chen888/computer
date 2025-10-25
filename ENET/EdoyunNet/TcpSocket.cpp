#include "TcpSocket.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

// 设置套接字非阻塞模式
void SocketUtil::SetNonBlock(int sockfd)
{
    int flags = fcntl(sockfd,F_GETFL,0);
    fcntl(sockfd,F_SETFL,flags | O_NONBLOCK);
}
// 设置套接字为阻塞模式
void SocketUtil::SetBlock(int sockfd)
{
    int flags = fcntl(sockfd,F_GETFL,0);
    fcntl(sockfd,F_SETFL,flags & (~O_NONBLOCK));
}
// 设置套接字复用，在服务器重启或快速重启时，避免因地址已被占用而无法绑定问题
void SocketUtil::SetReuseAddr(int sockfd)
{
    int on = 1;
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(const void*)&on,sizeof(on));
}
// 设置套接字复用，可以多个进程或线程同时监听同一个端口的场景
void SocketUtil::SetReusePort(int sockfd)
{
    int on = 1;
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEPORT,(const void*)&on,sizeof(on));
}

// 开启套接字的保活机制
void SocketUtil::SetKeepAlive(int sockfd)
{
    int on = 1;
    setsockopt(sockfd,SOL_SOCKET,SO_KEEPALIVE,(const void*)&on,sizeof(on));
}

// 设置套接字的发送缓冲区大小
void SocketUtil::SetSendBufSize(int sockfd, int size)
{
    setsockopt(sockfd,SOL_SOCKET,SO_SNDBUF,(const void*)&size,sizeof(size));
}
// 设置套接字的接受缓冲区大小
void SocketUtil::SetRecvBufSize(int sockfd, int size)
{
    setsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,(const void*)&size,sizeof(size));
}

TcpSocket::TcpSocket()
{
}

TcpSocket::~TcpSocket()
{
}

// 创建套接字 使用::是为了调用系统提供的socket函数
int TcpSocket::Create()
{
    sockfd_ = ::socket(AF_INET,SOCK_STREAM,0);
    return sockfd_;
}

// 绑定套接字到指定地址和端口
bool TcpSocket::Bind(std::string ip, short port)
{
    if(sockfd_ == -1)
    {
        return false;
    }
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);

    if(::bind(sockfd_,(struct sockaddr*)&addr,sizeof(addr)) == -1)
    {
        return false;
    }
    return true;;
}

// 监听套接字
bool TcpSocket::Listen(int backlog)
{
    if(sockfd_ == -1) 
    {
        return false;
    }
    if(::listen(sockfd_,backlog) == -1)
    {
        return false;
    }
    return true;
}

// 接受连接
int TcpSocket::Accept()
{
    struct sockaddr_in addr = {0};
    socklen_t addrlen = sizeof(addr);
    return ::accept(sockfd_,(struct sockaddr*)&addr,&addrlen);
}

// 关闭套接字
void TcpSocket::Close()
{
    if(sockfd_ != -1)
    {
        ::close(sockfd_);
        sockfd_ = -1;
    }
}

// 关闭套接字的写操作
void TcpSocket::ShutdownWrite()
{
    if(sockfd_ != -1)
    {
        shutdown(sockfd_,SHUT_WR);
        sockfd_ = -1;
    }
}
