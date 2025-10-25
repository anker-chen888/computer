#include "../EdoyunNet/BufferReader.h"

// 用于处理RTMP协议中的握手过程
class RtmpHandshake
{
public:
	enum State
	{
		HANDSHAKE_C0C1,	// 客户端发送
		HANDSHAKE_S0S1S2, // 服务端发送
		HANDSHAKE_C2, // 客户端发送
		HANDSHAKE_COMPLETE // 握手完成
	};

	RtmpHandshake(State state); // 构造函数
	virtual ~RtmpHandshake();
	// 解析接收到的数据并构建响应
	int Parse(BufferReader& in_buffer, char* res_buf, uint32_t res_buf_size);
	// 检查握手状态
	bool IsCompleted() const
	{
		return handshake_state_ == HANDSHAKE_COMPLETE;
	}
	// 构建C0C1数据包
	int BuildC0C1(char* buf,uint32_t buf_size);

private:
	State handshake_state_; // 当前握手状态
};