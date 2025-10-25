#include "../EdoyunNet/TcpConnection.h"
#include "define.h"

// 用于处理用户注册、登录和注销的逻辑
class LoginConnection : public TcpConnection
{
public:
    // 构造函数
    LoginConnection(TaskScheduler* scheduler,int socket);
    ~LoginConnection();
protected:
	bool IsTimeout(uint64_t timestamp); // 检查给定的时间戳是否超时
    bool OnRead(BufferReader& buffer);  // 处理接受到的数据
    void HandleMessage(BufferReader& buffer);   // 处理接受到的信息
    void Clear();   // 清理资源的函数
private:
    void HandleRegister(const packet_head* data);  // 处理用户注册请求
    void HandleLogin(const packet_head* data);     // 处理用户登录请求
    void HandleDestory(const packet_head* data);   // 处理用户注销请求
};