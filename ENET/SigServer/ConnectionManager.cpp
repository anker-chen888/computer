#include "ConnectionManager.h"

std::unique_ptr<ConnectionManager> ConnectionManager::instance_ = nullptr;

ConnectionManager::ConnectionManager()
{
}

ConnectionManager::~ConnectionManager()
{
    Close();
}

ConnectionManager *ConnectionManager::GetInstance()
{
    static std::once_flag flag;
    std::call_once(flag,[&](){
        instance_.reset(new ConnectionManager()); //只会创建一次
    });
    return instance_.get();
}
// 将一个新的用户连接添加到管理器中
void ConnectionManager::AddConn(const std::string &idefy, const TcpConnection::Ptr conn)
{
    if(idefy.empty())
    {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);   // 互斥锁
    auto it = connMaps_.find(idefy);
    if(it == connMaps_.end()) //说明这个连接器未加入
    {
        connMaps_.emplace(idefy,conn);
    }
}
// 移除一个用户
void ConnectionManager::RmvConn(const std::string &idefy)
{
    if(idefy.empty())
    {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    auto it = connMaps_.find(idefy);
    if(it != connMaps_.end())//查询到
    {
        connMaps_.erase(idefy);
    }
}

// 根据客户端的唯一标识查询对应的TcpConnection对象
TcpConnection::Ptr ConnectionManager::QueryConn(const std::string &idefy)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = connMaps_.find(idefy);
    if(it != connMaps_.end())
    {
        return it->second;
    }
    return nullptr;
}

void ConnectionManager::Close()
{
    connMaps_.clear();
}
