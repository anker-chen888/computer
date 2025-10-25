#include "LoginConnection.h"
#include "ORMManager.h"
#include <chrono>
#define TIMEOUT 60

LoginConnection::LoginConnection(TaskScheduler *scheduler, int socket)
    :TcpConnection(scheduler,socket)
{
    //
    this->SetReadCallback([this](std::shared_ptr<TcpConnection> conn,BufferReader& buffer){
        return this->OnRead(buffer);
    });
}

LoginConnection::~LoginConnection()
{
    Clear();
}

bool LoginConnection::OnRead(BufferReader &buffer)
{
    if(buffer.ReadableBytes() > 0)
    {
        HandleMessage(buffer);
    }
    return true;
}

// 处理消息逻辑
void LoginConnection::HandleMessage(BufferReader &buffer)
{
    if(buffer.ReadableBytes() < sizeof(packet_head))
    {
        return ;
    }
    packet_head* head = (packet_head*)buffer.Peek();
    if(buffer.ReadableBytes() < head->len)
    {
        return ;
    }
    switch (head->cmd)
    {
    case Login:
        HandleLogin((packet_head*)buffer.Peek()); // 处理登录要求
        break;
    case Register:
        HandleRegister((packet_head*)buffer.Peek()); // 处理注册要求
        break;
    case Destory:
        HandleDestory((packet_head*)buffer.Peek()); // 处理销毁要求
        break;
    default:
        break;
    }
    //更新buffer
    buffer.Retrieve(head->len);
}

// 判断是否超时
bool LoginConnection::IsTimeout(uint64_t timestamp) //true就是超时
{
    //获取当前时间
    auto now = std::chrono::system_clock::now();
    auto nowTimestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

    //计算差值
    int64_t time = nowTimestamp - timestamp;
    return abs(time) > TIMEOUT;
}

void LoginConnection::Clear()
{
}

// 处理用户注册请求
void LoginConnection::HandleRegister(const packet_head *data)
{
    RegisterResult reply;
    UserRegister* login = (UserRegister*)data; // 消息转化
    uint64_t time = login->timestamp;
    if(IsTimeout(time))
    {
        reply.resultCode = REQUEST_TIMEOUT;
    }
    else
    {
    std::string code = login->GetCode();
    //通过数据库来查询code是否存在
    MYSQL_ROW row = ORMManager::GetInstance()->UserLogin(code.c_str());
    if(row == NULL)//未注册
    {
        ORMManager::GetInstance()->UserRegister(login->GetName().c_str(),login->GetCount().c_str(),login->GetPasswd().c_str(),login->GetCode().c_str(),"192.168.31.30");
        reply.resultCode = S_OK;
    }
    else{
        reply.resultCode = ALREADY_REDISTERED;
    } 
    }
    this->Send((const char*)&reply,reply.len); // 将响应数据reply发送给数据
}

// 处理用户登录请求
void LoginConnection::HandleLogin(const packet_head *data)
{
    LoginResult reply;
    UserLogin* login = (UserLogin*)data;
    uint64_t time = login->timestamp;
    if(IsTimeout(time))
    {
        reply.resultCode = REQUEST_TIMEOUT;
    }
    else
    {
        //我们需要判断用户是否存在
        std::string code = login->GetCode();
        //通过数据库来查询code是否存在
        MYSQL_ROW row = ORMManager::GetInstance()->UserLogin(code.c_str());
        if(row == NULL) //用户未注册
        {
            reply.resultCode = SERVER_ERROR;
        }
        else
        {
            //判断用户是否一级登录
            if(atoi(row[4])) //在线
            {
                reply.resultCode = ALREADY_LOGIN;
            }
            else{
                reply.resultCode = S_OK;
                reply.SetIp("192.168.31.30");
                reply.port = 6539;
                //修改记录
                //我们先获取当前用户消息
                //获取当前时间
                auto now = std::chrono::system_clock::now();
                auto nowTimestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
                ORMManager::GetInstance()->insertClient(row[0],row[1],row[2],row[3],1,nowTimestamp,"192.168.31.30");
            }
        }
    }
    this->Send((const char*)&reply,reply.len);
}

void LoginConnection::HandleDestory(const packet_head *data)
{
    //注销
    UserDestory* destory = (UserDestory*)data;
    ORMManager::GetInstance()->UserDestroy(destory->GetCode().c_str());
}
