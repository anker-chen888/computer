#ifndef _RTMP_H_
#define _RTMP_H_
#include <cstdint>
#include <cstring>

// RTMP协议相关常量
static const int RTMP_VERSION = 0X3; // 协议版本号
static const int RTMP_SET_CHUNK_SIZE = 0x1; // 设置块大小的命令ID
static const int RTMP_ABORT_MESSAGE = 0x2;
static const int RTMP_ACK = 0x3;
static const int RTMP_ACK_SIZE = 0x5;
static const int RTMP_BANDWIDTH_SIZE = 0x6;
static const int RTMP_AUDIO = 0x08;
static const int RTMP_VIDEO = 0x09;
static const int RTMP_NOTIFY = 0x12;
static const int RTMP_INVOKE = 0x14;

//chunk类型 
static const int RTMP_CHUNK_TYPE_0 = 0; // 完整块头
static const int RTMP_CHUNK_TYPE_1 = 1; // 省略事件戳和信息长度
static const int RTMP_CHUNK_TYPE_2 = 2; // 省略事件戳、消息长度和消息类型ID
static const int RTMP_CHUNK_TYPE_3 = 3; // 仅包含流ID

//RTMP消息ID
static const int RTMP_CHUNK_CONTROL_ID = 2;  // 控制块的 ID
static const int RTMP_CHUNK_INVOKE_ID = 3; 	 // 命令调用块的 ID
static const int RTMP_CHUNK_AUDIO_ID = 4;   // 音频块的 ID
static const int RTMP_CHUNK_VIDEO_ID = 5;   // 视频块的 ID
static const int RTMP_CHUNK_DATA_ID = 6;   // 数据块的 ID

//编码类型
static const int RTMP_CODEC_ID_H264 = 7; // H.264视频编码的ID
static const int RTMP_CODEC_ID_AAC = 10; // AAC音频编码的ID

//元数据类型ID
static const int RTMP_AVC_SEQUENCE_HEADER = 0x18;  // AVC（H.264）序列头的 ID
static const int RTMP_AAC_SEQUENCE_HEADER = 0x19; // AAC 序列头的 ID


class Rtmp
{
public:
	virtual ~Rtmp() {};

	void SetChunkSize(uint32_t size)
	{
		if (size > 0 && size <= 60000) {
			max_chunk_size_ = size;
		}
	}

	void SetPeerBandwidth(uint32_t size)
	{ peer_bandwidth_ = size; }

	uint32_t GetChunkSize() const 
	{ return max_chunk_size_; }

	uint32_t GetAcknowledgementSize() const
	{ return acknowledgement_size_; }

	uint32_t GetPeerBandwidth() const
	{ return peer_bandwidth_; }

	// URL解析，提取IP地址、端口、应用名称和流名称等信息
	virtual int ParseRtmpUrl(std::string url)
	{
		char ip[100] = { 0 };
		char streamPath[500] = { 0 };
		char app[100] = { 0 };
		char streamName[400] = { 0 };
		uint16_t port = 0;

		if (strstr(url.c_str(), "rtmp://") == nullptr) {
			return -1;
		}

#if defined(__linux) || defined(__linux__)
		if (sscanf(url.c_str() + 7, "%[^:]:%hu/%s", ip, &port, streamPath) == 3)
#elif defined(WIN32) || defined(_WIN32)
		if (sscanf_s(url.c_str() + 7, "%[^:]:%hu/%s", ip, 100, &port, streamPath, 500) == 3)
#endif
		{
			port_ = port;
		}
#if defined(__linux) || defined(__linux__)
		else if (sscanf(url.c_str() + 7, "%[^/]/%s", ip, streamPath) == 2)
#elif defined(WIN32) || defined(_WIN32)
		else if (sscanf_s(url.c_str() + 7, "%[^/]/%s", ip, 100, streamPath, 500) == 2)
#endif
		{
			port_ = 1935;
		}
		else {
			return -1;
		}

		ip_ = ip;
		stream_path_ += "/";
		stream_path_ += streamPath;

#if defined(__linux) || defined(__linux__)
		if (sscanf(stream_path_.c_str(), "/%[^/]/%s", app, streamName) != 2)
#elif defined(WIN32) || defined(_WIN32)
		if (sscanf_s(stream_path_.c_str(), "/%[^/]/%s", app, 100, streamName, 400) != 2)
#endif
		{
			return -1;
		}

		app_ = app;
		stream_name_ = streamName;
		return 0;
	}
	// 流路径
	std::string GetStreamPath() const
	{ return stream_path_; }
	// 应用名称
	std::string GetApp() const
	{ return app_; }
	// 获取流名称
	std::string GetStreamName() const
	{ return stream_name_; }

	uint16_t port_ = 1935;  // RTMP 服务器的端口
	std::string ip_;	// 服务器的 IP 地址
	std::string app_;   // 应用名称。
	std::string stream_name_; // 流名称
	std::string stream_path_; // 流路径

	uint32_t peer_bandwidth_ = 5000000; // 对端带宽
	uint32_t acknowledgement_size_ = 5000000; // 确认信息大小
	uint32_t max_chunk_size_ = 128; // 最大块大小
};
#endif