#ifndef _TASKSCHDULER_H_
#define _TASKSCHDULER_H_
#include <cstdint>
#include "Timer.h"
#include "Channel.h"
#include <atomic>
#include <mutex>

// 是一个任务调度器，用户管理和调度任务，包括定时任务和I/O事件处理。
// 结合了TimerQueue和Channel的功能，能够在多线程环境中高效地处理各种事件。
class TaskScheduler
{
public:
    TaskScheduler(int id = 1);
    virtual ~TaskScheduler();
    void Start();
    void Stop();
    TimerId AddTimer(const TimerEvent& event, uint32_t mesc);
    void RemvoTimer(TimerId timerId);
    virtual void UpdateChannel(ChannelPtr channel){};
    virtual void RmoveChannel(ChannelPtr& channel){};
    virtual bool HandleEvent(){return false;} // 事件任务在epollTask中实现
    inline int GetId()const{return id_;}
private:
    int id_ = 0; // 任务调度器的标识符
    std::mutex mutex_;  // 互斥锁
    TimerQueue timer_queue_; // 定时器队列
    std::atomic_bool is_shutdown_; // 控制事件循环的停止
};
#endif