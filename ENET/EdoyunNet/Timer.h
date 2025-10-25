#include <map>
#include <unordered_map>
#include <thread>
#include <cstdint>
#include <functional>
#include <chrono>
#include <memory>

typedef std::function<bool(void)> TimerEvent;
typedef uint32_t TimerId;

// 表示单个定时器任务
class Timer
{
public: 
    Timer(const TimerEvent& event,uint32_t mesc)
    :evenrt_callbak_(event)
    ,interval_(mesc){}
    ~Timer(){}
    static void Sleep(uint32_t mesc)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(mesc));
    }
private:
    // 设置下一次超时时间
    void SetNextTimeOut(uint64_t time_point)
    {
        next_timeout_ = time_point + interval_;
    }
    // 获取下次超时时间
    int64_t getNextTimeOut()
    {
        return next_timeout_;
    }
private:
    friend class TimerQueue;   
    uint32_t interval_ = 0; // 定时器的时间间隔
    uint64_t next_timeout_ = 0;  // 定时器的下一次超时时间戳
    TimerEvent evenrt_callbak_ = []{return false;};  // 定时器触发时调用的回调函数
};

// 定时器队列，按照超时时间排序，并在适当的时候触发回调函数
class TimerQueue
{
public:
    TimerQueue(){}
    ~TimerQueue(){}
public:
    TimerId AddTimer(const TimerEvent& event,uint32_t mesc);  // 添加定时器
    void RemoveTimer(TimerId timerId);  // 移除指定定时器
    void HandleTimerEvent();    // 处理已到期的定时器事件
protected:
    int64_t GetTimeNow();
private:
    uint32_t last_timer_id_ = 0; // 生成唯一的定时器ID
    std::unordered_map<TimerId,std::shared_ptr<Timer>> timers_;  // 值为定时器ID，值为定时器的智能指针
    std::map<std::pair<int64_t,TimerId>, std::shared_ptr<Timer>> events_; // 确保了定时器按触发时间有序排列，map是有序排列的
};