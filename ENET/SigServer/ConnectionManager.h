#include <unordered_map>
#include <memory>
#include <mutex>
#include "../EdoyunNet/TcpConnection.h"

// 实习一个单例模式ConnectionManger类，用于管理用户连接。
class ConnectionManager
{
public:
    ~ConnectionManager();
    static ConnectionManager* GetInstance();
public:
    // 添加、移除连接
    void AddConn(const std::string& idefy,const TcpConnection::Ptr conn); // 添加要加标识符和连接业务
    void RmvConn(const std::string& idefy);
    TcpConnection::Ptr QueryConn(const std::string& idefy); // 查询客户端
    uint32_t Size()const{return connMaps_.size();}
private:
    ConnectionManager();
    void Close();
    std::mutex mutex_;
    static std::unique_ptr<ConnectionManager> instance_; // 唯一指针
    std::unordered_map<std::string,TcpConnection::Ptr> connMaps_;   // (客户端唯一标识符，TcpConnection对象)
};