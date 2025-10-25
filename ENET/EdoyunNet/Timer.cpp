#include "Timer.h"

TimerId TimerQueue::AddTimer(const TimerEvent &event, uint32_t mesc)
{
    int64_t time_point = GetTimeNow(); // 获取当前时间戳，以毫秒为单位
    TimerId timer_id = ++last_timer_id_; // 通过++生成唯一id

    auto timer = std::make_shared<Timer>(event,mesc);
    timer->SetNextTimeOut(time_point);
    timers_.emplace(timer_id,timer); // 添加到unorder_map
    events_.emplace(std::pair<int64_t,TimerId>(time_point + mesc,timer_id),timer); // 下次超时时间 定时器ID 和定时器对象
    return timer_id;
}

void TimerQueue::RemoveTimer(TimerId timerId)
{
    auto iter = timers_.find(timerId);
    if(iter != timers_.end())
    {
        int64_t timeout = iter->second->getNextTimeOut();
        events_.erase(std::pair<int64_t,TimerId>(timeout,timerId));
        timers_.erase(timerId);
    }
}
// 检查定时器队列中哪些定时器的触发时间已到达，调用对应的回调函数。如果是一次性的，处理完移除；
// 如果是重复的，则重新计算下一次触发时间，并插入队列
void TimerQueue::HandleTimerEvent()
{
    if(!timers_.empty())
    {
        int64_t timepoint = GetTimeNow(); // 获取当前时间
        while(!timers_.empty() && events_.begin()->first.first <= timepoint)
        {
            TimerId timerId = events_.begin()->first.second;
            if(events_.begin()->first.second)
            {
                bool flag = events_.begin()->second->evenrt_callbak_();
                if(flag) //反复执行
                {
                    // 通过std::move将执行完的定时器任务重新移动，然后添加到map中
                    events_.begin()->second->SetNextTimeOut(timepoint);
                    auto timePtr = std::move(events_.begin()->second);
                    events_.erase(events_.begin());
                    events_.emplace(std::pair<int64_t,TimerId>(timePtr->getNextTimeOut(),timerId),timePtr);
                }
                else //一次性
                {
                    events_.erase(events_.begin());
                    timers_.erase(timerId);
                } 
            }
        }
    }
}

int64_t TimerQueue::GetTimeNow()
{
    auto time_point = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(time_point.time_since_epoch()).count();
}
