#include <functional>
#include <memory>
#include "Channel.h"
#include "TcpSocket.h"

class EventLoop;

typedef std::function<void(int)> NewConnectCallback;

// 用于处理 TCP 服务端的连接接受操作
class Acceptor
{
public:
    Acceptor(EventLoop* eventloop);
    ~Acceptor();
    inline void SetNewConnectCallback(const NewConnectCallback& cb) {new_connectCb_ = cb;};
    int Listen(std::string ip,uint16_t port);
    void Close();
private:
    void OnAccept();
    EventLoop* loop_ = nullptr;     // 事件处理
    ChannelPtr channelPtr_ = nullptr;   // 用于管理套接字的事件监听
    std::shared_ptr<TcpSocket> tcp_socket_; // tcpSocket
    NewConnectCallback new_connectCb_;  // 新连接回调函数
};