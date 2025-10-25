#include "define.h"

// 能够连接到负载均衡服务器，获取系统内存使用该情况，
// 并将内存使用信息发送到服负载均衡服务器
class TcpClient
{
public:
	TcpClient();
	void Create();
	// 连接到指定的服务器IP和端口
	bool Connect(std::string ip, uint16_t port);
	void Close();
	~TcpClient();
	// 用于获取监控信息，包括内存使用情况
    void getMonitorInfo();  
protected:
	// 用于获取系统内存使用情况
    void get_mem_usage();
	// 发送数据到服务器
	void Send(uint8_t* data, uint32_t size);
private:
	FILE* file_;	// 打开和读取/proc/meminfo文件的文件指针
	bool isConnect_;	// 标记客户端是否已连接到服务器
	int sockfd_;	// TCP套接字描述符
	struct sysinfo info_;
	Monitor_body Monitor_info_;		// 存储监控信息的对象
};