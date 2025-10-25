#include "TcpConnection.h"
#include <unistd.h>
#include "Channel.h"
// 初始化一个 TcpConnection 对象，设置其相关的回调函数，
// 配置套接字属性，并将通道注册到任务调度器中进行事件监听。
TcpConnection::TcpConnection(TaskScheduler *task_schduler, int sockfd)
    :task_schduler_(task_schduler)
    ,read_buffer_(new BufferReader())
    ,write_buffer_(new BufferWriter(500))
    ,channel_(new Channel(sockfd))
{
    is_closed_ = false;
    // 设置通道的相应回调函数
    channel_->SetReadCallback([this](){this->HandleRead();});
    channel_->SetWriteCallback([this](){this->HandleWrite();});
    channel_->SetCloseCallback([this](){this->HandleClose();});
    channel_->SetErrorCallback([this](){this->HandleError();});

    //设置套接字属性
    SocketUtil::SetNonBlock(sockfd);
    SocketUtil::SetSendBufSize(sockfd,100 * 1024);
    SocketUtil::SetKeepAlive(sockfd);

    // 开启读时间监听并更新通道任务
    channel_->EnableReading();
    task_schduler_->UpdateChannel(channel_);    
}

TcpConnection::~TcpConnection()
{
    int fd = channel_->GetSocket();
    if(fd > 0)
    {
        ::close(fd);
    }
}
// 进行数据的发送
void TcpConnection::Send(std::shared_ptr<char> data, uint32_t size)
{
    if(!is_closed_)
    {
        mutex_.lock();
        write_buffer_->Append(data,size);
        mutex_.unlock();
        this->HandleWrite();
    }
}

void TcpConnection::Send(const char *data, uint32_t size)
{
    if(!is_closed_)
    {
        mutex_.lock();
        write_buffer_->Append(data,size);
        mutex_.unlock();
        this->HandleWrite();
    }
}

void TcpConnection::DisConnect()
{
    std::lock_guard<std::mutex> lock(mutex_);
    this->Close();
}
// 将套接字读取到的数据存储到读缓冲区中，并调用用户设置的读回调函数进行处理
void TcpConnection::HandleRead()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if(is_closed_)
        {
            return;
   	    }
        int ret = read_buffer_->Read(channel_->GetSocket());
        if(ret < 0)
   	    {
            this->Close();
            return;
        } //这个锁会释放掉
    }
    if(readCb_)
    {
        bool ret = readCb_(shared_from_this(),*read_buffer_); // 读取到数据后进行回调函数处理
        if(!ret)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            this->Close();
        }
    }
}

// 将写缓冲区中的数据发送到对端
void TcpConnection::HandleWrite()
{
    if(is_closed_)
    {
        return;
    }
    //获取锁
    if(!mutex_.try_lock())
    {
        return ;
    }
    int ret = 0;
    bool empty = false;
    do
    {
        ret = write_buffer_->Send(channel_->GetSocket());
        if(ret < 0)
        {
            this->Close();
            mutex_.unlock();
            return;
        }
        empty = write_buffer_->IsEmpty();
    }while (0);

    if(empty)
    {
        if(channel_->IsWriting())
        {
            channel_->DisableWriting();
            task_schduler_->UpdateChannel(channel_);
        }
    }
    // 缓冲区不为空，如果通道不处于写事件监听状态，启用写事件监听，确保后续数据能够继续发送
    else if(!channel_->IsWriting())
    {
        channel_->EnableWriting();
        task_schduler_->UpdateChannel(channel_);
    }
    mutex_.unlock();
}

// 处理关闭事件
void TcpConnection::HandleClose()
{
    std::lock_guard<std::mutex> lock(mutex_);
    this->Close();
}

void TcpConnection::HandleError()
{
    std::lock_guard<std::mutex> lock(mutex_);
    this->Close();
}
// 关闭当前TCP连接，清理相关资源
void TcpConnection::Close()
{
    if(!is_closed_)
    {
        is_closed_ = true;
        task_schduler_->RmoveChannel(channel_);
        if(closeCb_)
        {
            closeCb_(shared_from_this());
        }
        if(disconnectCb_)
        {
            disconnectCb_(shared_from_this());
        }
    }
}
