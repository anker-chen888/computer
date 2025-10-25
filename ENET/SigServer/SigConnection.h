#include <map>
#include <vector>
#include <cstdint>
#include "define.h"
#include "../EdoyunNet/TcpConnection.h"

// 管理信令连接的类
class SigConnection : public TcpConnection
{
public:
    SigConnection(TaskScheduler* scheduler,int socket); // 接受一个任务调度器指针和一个套接字描述符
    ~SigConnection();
public:
    // 提供客户端连接状态的检查
    bool IsAlive(){return state_ != CLOSE;} // 是否连接
    bool IsNoJion(){return state_ == NONE;} // 是否加入房间
    bool IsIdle(){return state_ == IDLE;};  // 是否空闲
    bool IsBusy(){return (state_ == PUSHER || state_ == PULLER);}; // 是否处于推流或拉流状态

    void DisConnected();    // 断开连接
    // 添加或移除自定义标识符
    void AddCustom(const std::string& code);
    void RmoveCustom(const std::string& code);

    RoleState GetRoleState()const{return state_;};  // 获取客户端状态
    std::string GetCode()const{return code_;};  // 获取标识符
    std::string GetStreamAddres()const{return streamAddres_;}; // 获取流地址
protected:
    // 处理从套接字读取的数据，解析和处理接受到的信息
    bool OnRead(BufferReader& buffer);
    void HandleMessage(BufferReader& buffer);
    void Clear();
private:
    // 处理不同类型的信令消息
    void HandleJion(const packet_head* data);
    void HandleObtainStream(const packet_head* data);
    void HandleCreateStream(const packet_head* data);
    void HandleDeleteStream(const packet_head* data);
    void HandleOtherMessage(const packet_head* data);
private:
    // 实现获取流和创建流的具体逻辑
    void DoObtainStream(const packet_head* data);
    void DoCreateStream(const packet_head* data);
private:
    RoleState state_;   // 客户端状态
    std::string code_;  // 客户端唯一标识符
    std::string streamAddres_;  // 流地址
    TcpConnection::Ptr conn_ = nullptr;   // 用于数据传输
    std::vector<std::string> objectes_;   // 存储客户端标识符
};