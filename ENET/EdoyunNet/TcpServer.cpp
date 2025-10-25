#include "TcpServer.h"
#include "Acceptor.h"
#include "EventLoop.h"

TcpServer::TcpServer(EventLoop *eventloop)
    :loop_(eventloop)
    ,port_(0)
    ,acceptor_(new Acceptor(eventloop))
{
    // 有新的连接调用这个
    acceptor_->SetNewConnectCallback([this](int fd){
        TcpConnection::Ptr conn = this->OnConnect(fd); // 有新的连接创建一个TcpConnection管理
        if(conn)
        {
            this->AddConnection(fd,conn); // 添加到映射表
            conn->SetDisConnectCallback([this](TcpConnection::Ptr conn){
                int fd = conn->GetSocket();
                this->RemoveConnection(fd);
            });
        }
    });
}

TcpServer::~TcpServer()
{
    Stop();
}

// 启动服务器
bool TcpServer::Start(std::string ip, uint16_t port)
{
    Stop();
    if(!is_stared_)
    {
        if(acceptor_->Listen(ip,port) < 0)
        {
            return false;
        }
        port_ = port;
        ip_ = ip;
        is_stared_ = true;
    }
    return true;
}

// 停止服务器
void TcpServer::Stop()
{
    if(is_stared_)
    {
        for(auto iter : connects_)
        {
            iter.second->DisConnect();
        }

        acceptor_->Close();
        is_stared_ = false;
    }
}

// 当有新连接时，创建一个新的TcpConnection对象
TcpConnection::Ptr TcpServer::OnConnect(int fd)
{
    return std::make_shared<TcpConnection>(loop_->GetTaskSchduler().get(),fd);
}

// 将新连接添加到活动连接映射表中
void TcpServer::AddConnection(int fd, TcpConnection::Ptr conn)
{
    connects_.emplace(fd,conn); // 插入到unorder_map中进行管理
}

// 从活动连接映射表中移除指定的连接
void TcpServer::RemoveConnection(int fd)
{
    connects_.erase(fd);
}