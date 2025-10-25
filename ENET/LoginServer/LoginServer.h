#include "../EdoyunNet/TcpServer.h"
#include "TcpClient.h"

// 实现一个登录服务器的创建和初始化逻辑
class LoginServer : public TcpServer
{
public:
   static std::shared_ptr<LoginServer> Create(EventLoop* eventloop); 
   ~LoginServer();
private:
   TimerId id_;   // 定时器ID
   EventLoop* loop_; // 用于时间循环和任务调度
   LoginServer(EventLoop* eventloop); // 构造函数
   virtual TcpConnection::Ptr OnConnect(int socket);  
   std::unique_ptr<TcpClient> client_;    // 与外部连接
};