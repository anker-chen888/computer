#ifndef _TCPCONNECTION_H_
#define _TCPCONNECTION_H_
#include "BufferReader.h"
#include "BufferWriter.h"
#include "Channel.h"
#include "TcpSocket.h"
#include "TaskScheduler.h"

// 对象抽象会话场景，确保每个 TCP 连接都有一个对应的对象来处理所有相关的业务逻辑
class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    using Ptr = std::shared_ptr<TcpConnection>;
    using DisConnectCallback = std::function<void(std::shared_ptr<TcpConnection>)>;
    using CloseCallback = std::function<void(std::shared_ptr<TcpConnection>)>;
    using ReadCallback = std::function<bool(std::shared_ptr<TcpConnection>,BufferReader& buffer)>;
    // 通过外部传入的sockfd来创建channel对象，添加到taskschedule对象中进行事件监听和处理
    TcpConnection(TaskScheduler* task_schduler,int sockfd);
    virtual ~TcpConnection();
    // 返回当前
    inline TaskScheduler* GetTaskSchduler()const{return task_schduler_;}
    // 设置回调函数
    inline void SetReadCallback(const ReadCallback& cb){readCb_ = cb;}
    inline void SetCloseCallback(const CloseCallback& cb){closeCb_ = cb;} // 当连接关闭触发回调函数
    // 连接状态
    inline bool IsClosed()const{return is_closed_;}
    inline int GetSocket()const{return channel_->GetSocket();}
    
    void Send(std::shared_ptr<char> data,uint32_t size);
    void Send(const char* data,uint32_t size);
    void DisConnect();
protected:
    virtual void HandleRead();  // 处理读事件，将套接字读取缓冲区数据拷贝到应用层缓冲区
    virtual void HandleWrite(); // 处理写事件，将用户数据拷贝到发送缓冲区
    virtual void HandleClose(); // 处理套接字关闭事件
    virtual void HandleError(); // 处理网络错误事件
protected:
    void SetDisConnectCallback(const DisConnectCallback& cb) {disconnectCb_ = cb;} // 设置断开连接回调函数
    friend class TcpServer; // 友元类
    bool is_closed_;    // 连接是否关闭
    TaskScheduler* task_schduler_;  // 任务调度器
    std::unique_ptr<BufferReader> read_buffer_; // 读缓冲区
    std::unique_ptr<BufferWriter> write_buffer_; // 写缓冲区
private:
    void Close();
    std::mutex mutex_; // 互斥量
    std::shared_ptr<Channel> channel_ = nullptr;  // 套接字的事件设置
    DisConnectCallback disconnectCb_;   // 断开连接回调函数
    CloseCallback closeCb_; // 关闭连接回调函数
    ReadCallback readCb_; // 读取数据的回调函数
};
#endif