#ifndef _TCPSERVER_H_
#define _TCPSERVER_H_
#include <memory>
#include <string>
#include <unordered_map>
#include "TcpConnection.h"

class EventLoop;
class Acceptor;

//TCP服务器的核心实现，负责管理客户端连接、启动和停止服务器。
class TcpServer
{
public:
    TcpServer(EventLoop* eventloop);
    ~TcpServer();
    virtual bool Start(std::string ip,uint16_t port);
    virtual void Stop();
    inline  std::string GetIPAddres()const{return ip_;}
    inline  uint16_t GetPort()const{return port_;}
protected:
    virtual TcpConnection::Ptr OnConnect(int fd);
    virtual void AddConnection(int fd,TcpConnection::Ptr conn);
    virtual void RemoveConnection(int fd);
private:
    EventLoop* loop_; // 执行循环
    uint16_t port_;     // 端口
    std::string ip_;    // ip
    std::unique_ptr<Acceptor> acceptor_;    // Acceptor对象的函数
    bool is_stared_ = false; // 服务器是否已启动
    std::unordered_map<int,TcpConnection::Ptr> connects_; // 存储所有活动连接的映射
};
#endif