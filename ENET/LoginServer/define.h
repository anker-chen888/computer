#ifndef _DEFINE_H_
#define _DEFINE_H_
#include <cstdint>
#include <string>
#include <array>
#include <sys/sysinfo.h>

// 定义了用于网络通信的消息结构体和相关枚举类型
#pragma pack(push,1)

// 给出不同的命令类型
enum Cmd : uint16_t
{
    Minotor,
    ERROR,
    Login,
    Register,
    Destory,
};

// 操作的结果代码
enum ResultCode
{
	S_OK = 0,
	SERVER_ERROR ,
	REQUEST_TIMEOUT ,
	ALREADY_REDISTERED ,
	USER_DISAPPEAR,
	ALREADY_LOGIN,
	VERFICATE_FAILED
};

// 基础的消息头结构
struct packet_head {
    packet_head()
        :len(-1)
        , cmd(-1) {}
    uint16_t len;
    uint16_t cmd;
};

// 用户注册消息结构体
struct UserRegister : public packet_head
{
    UserRegister():packet_head()
    {
        cmd = Register;
        len = sizeof(UserRegister);
    }
    void SetCode(const std::string& str)
    {
        str.copy(code.data(),code.size(),0);
    }
    std::string GetCode()
    {
        return std::string(code.data());
    }
    void SetName(const std::string& str)
    {
        str.copy(name.data(),name.size(),0);
    }
    std::string GetName()
    {
        return std::string(name.data());
    }
    void SetCount(const std::string& str)
    {
        str.copy(count.data(),count.size(),0);
    }
    std::string GetCount()
    {
        return std::string(count.data());
    }
    void SetPasswd(const std::string& str)
    {
        str.copy(passwd.data(),passwd.size(),0);
    }
    std::string GetPasswd()
    {
        return std::string(passwd.data());
    }
    std::array<char,20> code;   // 验证码
    std::array<char,20> name;   // 用户名
    std::array<char,12> count;  // 用户计数
    std::array<char,20> passwd; // 存储用户密码
    uint64_t timestamp;         // 存储时间戳
};

// 用户登录消息结构体
struct UserLogin : public packet_head
{
    UserLogin():packet_head()
    {
        cmd = Login;
        len = sizeof(UserLogin);
    }
    void SetCode(const std::string& str)
    {
        str.copy(code.data(),code.size(),0);
    }
    std::string GetCode()
    {
        return std::string(code.data());
    }
    void SetCount(const std::string& str)
    {
        str.copy(count.data(),count.size(),0);
    }
    std::string GetCount()
    {
        return std::string(count.data());
    }
    void SetPasswd(const std::string& str)
    {
        str.copy(passwd.data(),passwd.size(),0);
    }
    std::string GetPasswd()
    {
        return std::string(passwd.data());
    }
    std::array<char,20> code;
    std::array<char,12> count;
    std::array<char,33> passwd; //Md5
    uint64_t timestamp;
};

// 注册结果消息结构体
struct RegisterResult : public packet_head
{
    RegisterResult():packet_head()
    {
        cmd = Register;
        len = sizeof(RegisterResult);
    }
	ResultCode resultCode;
};

// 登录结果消息结构体
struct LoginResult : public packet_head
{
    LoginResult() : packet_head()
    {
        cmd = Login;
        len = sizeof(LoginResult);
    }
    void SetIp(const std::string& str)
    {
        str.copy(ctrSvrIp.data(),ctrSvrIp.size(),0);
    }
    std::string GetIp()
    {
        return std::string(ctrSvrIp.data());
    }
	ResultCode resultCode;
	uint16_t port;
	std::array<char, 16> ctrSvrIp;
};

// 用户注销消息结构体
struct UserDestory : public packet_head
{
    UserDestory(): packet_head()
    {
        cmd = Destory;
        len = sizeof(UserDestory);
    }
    void SeCode(const std::string& str)
    {
        str.copy(code.data(),code.size(),0);
    }
    std::string GetCode()
    {
        return std::string(code.data());
    }
    std::array<char,20> code;
};

// 监控消息结构体
struct Monitor_body : public packet_head {
    Monitor_body()
        :packet_head()
    {
        cmd = Minotor;
        len = sizeof(Monitor_body);
        ip.fill('\0');
    }
    void SetIp(const std::string& str)
    {
        str.copy(ip.data(), ip.size(), 0);
    }
    std::string GetIp()
    {
        return std::string(ip.data());
    }
    uint8_t mem;    // 内存使用信息
    std::array<char, 16> ip;    // 存储IP地址
	uint16_t port;  // 存储端口号
};
#pragma pack(pop)
#endif