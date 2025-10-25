#include "EpoolTaskScheduler.h"
#include <vector>

// 负责管理事件循环、线程池、任务调度以及定时器等功能
// 大致流程
// 根据用户选择来启动对应的线程数，直接每一个线程均由时间对象TaskSchedule来处理
// 它作为一个基类，具体的事件处理细节由其子类来实现
// 为每个TaskSchedule都绑定了一个线程，充分利用了多核CPU的能力，每一个核的线程负责监听一组文件描述符的集合和事件处理
class EventLoop
{
public:
    EventLoop(uint32_t num_threads = -1);
    ~EventLoop();
    // 禁止拷贝或赋值
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator = (const EventLoop&) = delete;
    std::shared_ptr<TaskScheduler> GetTaskSchduler();

    // 添加定时任务
    TimerId AddTimer(const TimerEvent& event,uint32_t mesc);
    void RemvoTimer(TimerId timerId);

    void UpdateChannel(ChannelPtr channel);
    void RmoveChannel(ChannelPtr& channel);
    void Loop();
    void Quit();
private:
    uint32_t num_threads_ = 1; // 线程数量
    uint32_t index_ = 1; // 用于表示当前调度器的索引
    std::vector<std::shared_ptr<TaskScheduler>> task_schdulers_; // 存储多个TaskScheduler对象的智能指针，每个调度器管理一组任务
    std::vector<std::shared_ptr<std::thread>> threads_;  // 线程池
};