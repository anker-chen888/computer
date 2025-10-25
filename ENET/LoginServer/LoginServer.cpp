#include "LoginServer.h"
#include "../EdoyunNet/EventLoop.h"
#include "LoginConnection.h"

std::shared_ptr<LoginServer> LoginServer::Create(EventLoop *eventloop)
{
    std::shared_ptr<LoginServer> server(new LoginServer(eventloop));
    return server;
}

LoginServer::LoginServer(EventLoop* eventloop)
    :TcpServer(eventloop)
    ,loop_(eventloop)
    ,client_(nullptr)
{
    client_.reset(new TcpClient());
    client_->Create();
    // 每秒发送一次监控信息
    if(client_->Connect("192.168.31.30",8523))
    {
        client_->getMonitorInfo();
        id_ = loop_->AddTimer([this](){
            client_->getMonitorInfo();
            return true;
        },1000);    // 每秒调用一次监控消息
    }
}

LoginServer::~LoginServer()
{
}

TcpConnection::Ptr LoginServer::OnConnect(int socket)
{
    return std::make_shared<LoginConnection>(loop_->GetTaskSchduler().get(),socket);
}