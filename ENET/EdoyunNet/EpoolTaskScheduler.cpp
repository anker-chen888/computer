#include "EpoolTaskScheduler.h"
#include <sys/epoll.h>
#include <errno.h>
#include <iostream>

// 初始化任务调度器ID
EpollTaskScheduler::EpollTaskScheduler(int id)
    :TaskScheduler(id)
{
    //创建epoll 可监听1024个套接字
    epollfd_ = epoll_create(1024);
}

EpollTaskScheduler::~EpollTaskScheduler()
{
}

// 更新通道在epoll实例中的注册状态
void EpollTaskScheduler::UpdateChannel(ChannelPtr channel)
{
    std::lock_guard<std::mutex> lock(mutex_);
    int fd = channel->GetSocket();
    if(channels_.find(fd) != channels_.end())
    {
        if(channel->IsNoneEvent())
        {
            Update(EPOLL_CTL_DEL,channel);
            channels_.erase(fd);
        }
        else{
            Update(EPOLL_CTL_MOD,channel);
        }
    }
    else
    {
        if(!channel->IsNoneEvent())
        {
            channels_.emplace(fd,channel);
            Update(EPOLL_CTL_ADD,channel);
        }
    }
}

// 从epoll实例和channels_中移除指定的通道
void EpollTaskScheduler::RmoveChannel(ChannelPtr &channel)
{
    std::lock_guard<std::mutex> lock(mutex_);
    int fd = channel->GetSocket();
    if(channels_.find(fd) != channels_.end())
    {
        Update(EPOLL_CTL_DEL,channel);
        channels_.erase(fd);
    }
}

//  处理事件
bool EpollTaskScheduler::HandleEvent()
{
    struct epoll_event events[512] = {0};
    int num_events = -1;

    num_events = epoll_wait(epollfd_,events,512,0); // 等待实例中的事件发送
    if(num_events < 0)
    {
        if(errno != EINTR)
        {
            return false;
        }
    }
    // 遍历返回的事件列表看，调用相应通道的HandleEvent方法处理事件
    for(int n = 0; n < num_events; n++)
    {
        if(events[n].data.ptr) // 检查数据
        {
            ((Channel*)events[n].data.ptr)->HandleEvent(events[n].events); // 执行对应的事件任务
        }
    }
    return true;
}

// 用于向epoll实例中添加、修改或删除通道
void EpollTaskScheduler::Update(int operation, ChannelPtr &Channel)
{
    struct epoll_event event = {0};
    if(operation != EPOLL_CTL_DEL)
    {
        event.data.ptr = Channel.get();
        event.events = Channel->GetEvents();
    }

    if(::epoll_ctl(epollfd_,operation,Channel->GetSocket(),&event) < 0)
    {
        std::cout << "修改epoll事件失败";
    }
}
