#ifndef _ORMMANAGER_H_
#define _ORMMANAGER_H_
#include <memory>
#include <mutex>
#include <mysql/mysql.h>

// 实现了一个单例模式的ORMManger类，
// 用于管理用户注册、登录和注销等操作，并通过MySQL进行数据存储和查询
class ORMManager
{
public:
    ~ORMManager();
    static ORMManager* GetInstance(); //全局实例
    // 用户注册，将用户信息插入到数据库中
    void UserRegister(const char* name, const char* acount, const char* password, const char* usercode,const char* sig_server);
    // 用户登录。通过用户识别码查询数据库，返回用户信息
    MYSQL_ROW UserLogin(const char* usercode);
    // 用户注销
    void UserDestroy(const char* usercode);
	// 客户端信息插入到数据库中
    void insertClient(const char* name, const char* acount, const char* password, const char* usercode, int online, long recently_login, const char* sig_server);
protected:
 	void deleteClientByUsercode(const char* usercode);
	MYSQL_ROW selectClientByUsercode(const char* usercode);
private:
    ORMManager();   // 私有构造函数
    MYSQL mysql_;   // 用于与MySQL数据库的连接
private:
    static std::unique_ptr<ORMManager> instance_;
};
#endif