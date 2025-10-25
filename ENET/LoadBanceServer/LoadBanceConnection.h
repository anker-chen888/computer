#include "../EdoyunNet/TcpConnection.h"
#include "LoadBanceServer.h"

// 用于处理负载均衡连接的网络连接类
class LoadBanceConnection : public TcpConnection
{
public:
	LoadBanceConnection(std::shared_ptr<LoadBanceServer> loadbance_server,TaskScheduler* task_scheduler, int sockfd);
	~LoadBanceConnection();
protected:
    void DisConnection();					// 处理断开连接的逻辑
	bool OnRead(BufferReader& buffer);		// 有数据可读时调用的函数，转而交给HandleMessage
	bool IsTimeout(uint64_t timestamp);		// 检查给定的时间戳是否超时
	void HandleMessage(BufferReader& buffer); // 处理接受到的消息
	void HnadleLogin(BufferReader& buffer);	// 处理登录请求
	void HandleMinoterInfo(BufferReader& buffer);	// 处理监控消息
private:
	int socket_;	// 存储套接字描述符
	std::weak_ptr<LoadBanceServer> loadbance_server_;	// 存储指向Load...对象的弱引用
};