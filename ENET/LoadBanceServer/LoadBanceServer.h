#ifndef LOADBANCESERVER_H_
#define LOADBANCESERVER_H_
#include "../EdoyunNet/TcpServer.h"
#include "loaddefine.h"

// 实现了一个负载均衡服务器类LoadBanceServer，它通过继承TcpServer处理网络连接
// 并在内部维护了一个监控信息表monitorInfos_。当有新的连接时，它创建LoadBanceConnection对象来处理连接
class LoadBanceServer : public TcpServer, public std::enable_shared_from_this<LoadBanceServer>
{
public:
    static std::shared_ptr<LoadBanceServer> Create(EventLoop* eventloop);  //设单例
    ~LoadBanceServer();
private:
    friend class LoadBanceConnection;
    LoadBanceServer(EventLoop* eventloop);
    virtual TcpConnection::Ptr OnConnect(int socket);
	void UpdateMonitor(const int fd, Monitor_body* info);
	Monitor_body* GetMonitorInfo();
private:
    EventLoop* loop_;   // 用于时间循环和任务调度
    std::mutex mutex_;
    std::map<int, Monitor_body*> monitorInfos_; // 存储监控信息的映射表，键为文件描述符，值为Monitor...对象的指针
};
#endif