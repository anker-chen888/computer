#ifndef _CHANNEL_H_
#define _CHANNEL_H_
#include <functional>
#include <memory>

enum EventType
{
	EVENT_NONE   = 0,
	EVENT_IN     = 1,
	EVENT_PRI    = 2,		
	EVENT_OUT    = 4,
	EVENT_ERR    = 8,
	EVENT_HUP    = 16,
};

// 对文件描述符fd的封装，用于管理fd的事件监听和处理。
// 将fd及其感兴趣的事件注册到事件监听器（如epoll）上，并在事件发生时调用相应的处理函数
class Channel 
{
public:
	typedef std::function<void()> EventCallback; // 函数指针类型，表示事件处理函数
	Channel(int sockfd) 
		: sockfd_(sockfd){}
		
	~Channel() {};
    
	inline void SetReadCallback(const EventCallback& cb)
	{ read_callback_ = cb; }

	inline void SetWriteCallback(const EventCallback& cb)
	{ write_callback_ = cb; }

	inline void SetCloseCallback(const EventCallback& cb)
	{ close_callback_ = cb; }

	inline void SetErrorCallback(const EventCallback& cb)
	{ error_callback_ = cb; }

	inline int GetSocket() const { return sockfd_; } // 返回1当前Channel对象管理的文件描述符

	inline int  GetEvents() const { return events_; }
	// 根据传入的events参数，检查是否发生了读、写、关闭与错误事件
	inline void SetEvents(int events) { events_ = events; }
    
	inline void EnableReading() 
	{ events_ |= EVENT_IN; }

	inline void EnableWriting() 
	{ events_ |= EVENT_OUT; }
    
	inline void DisableReading() 
	{ events_ &= ~EVENT_IN; }
    
	inline void DisableWriting() 
	{ events_ &= ~EVENT_OUT; }

	// 检查是否启用读写事件
	inline bool IsNoneEvent() const { return events_ == EVENT_NONE; }
	inline bool IsWriting() const { return (events_ & EVENT_OUT) != 0; }
	inline bool IsReading() const { return (events_ & EVENT_IN) != 0; }
    
	// 处理发生的事件
	void HandleEvent(int events)
	{	
		if (events & (EVENT_PRI | EVENT_IN)) {
			read_callback_();
		}

		if (events & EVENT_OUT) {
			write_callback_();
		}
        
		if (events & EVENT_HUP) {
			close_callback_();
			return ;
		}

		if (events & (EVENT_ERR)) {
			error_callback_();
		}
	}

private:
	// 一个文件描述符发生可读、可写、关闭、错误事件，使用以下函数处理
	EventCallback read_callback_  = []{}; // 默认为一个空的Lambda表达式
	EventCallback write_callback_ = []{};
	EventCallback close_callback_ = []{};
	EventCallback error_callback_ = []{};
	int sockfd_ = 0;	//照看的文件描述符
	int events_ = 0;    // fd感兴趣的事件类型集合
};

typedef std::shared_ptr<Channel> ChannelPtr; // 定义智能指针，方便管理Channel对象周期
#endif