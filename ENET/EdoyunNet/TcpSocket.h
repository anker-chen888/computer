#ifndef _TCPSOCKET_H_
#define _TCPSOCKET_H_
#include <string>

// Socket属性设置类
// 1.阻塞/非阻塞模式 2. 地址重用  3.TCP保活  4.设置发送/接受缓冲区大小
class SocketUtil
{
public:
    static void SetNonBlock(int sockfd);
    static void SetBlock(int sockfd);
    static void SetReuseAddr(int sockfd);
    static void SetReusePort(int sockfd);
    static void SetKeepAlive(int sockfd);
    static void SetSendBufSize(int sockfd, int size);
    static void SetRecvBufSize(int sockfd, int size);
};

// Tcp网络连接类
class TcpSocket
{
public:
    TcpSocket();
    virtual ~TcpSocket();
    int Create();
    bool Bind(std::string ip, short port);
    bool Listen(int backlog);
    int  Accept();
    void Close();
    void ShutdownWrite();
    int GetSocket() const { return sockfd_; }
private:
    int sockfd_ = -1;
};
#endif