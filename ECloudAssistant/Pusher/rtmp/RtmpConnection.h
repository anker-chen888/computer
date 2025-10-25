#ifndef RTMPCONNECTION
#define RTMPCONNECTION
#include "TcpConnection.h"
#include "amf.h"
#include "rtmp.h"
#include "RtmpChunk.h"
#include "RtmpHandshake.h"

class RtmpPublisher;
// 连接状态
enum ConnectionState
{
    HANDSHAKE, // 握手
    START_CONNECT, // 开始连接
    START_CREATE_STREAM, // 创建流
    START_DELETE_STREAM, // 删除流
    START_PUBLISH, // 开始推流
};

class RtmpPublisher;
class RtmpConnection : public TcpConnection
{
public:
    RtmpConnection(std::shared_ptr<RtmpPublisher> publisher, TaskScheduler* scheduler, int sockfd);
    virtual ~RtmpConnection();

    bool OnRead(BufferReader& buffer); // 处理从网络读取的数据
    void OnClose();

    bool HandleChunk(BufferReader& buffer); // 解析和处理RTMP块数据
    bool HandleMessage(RtmpMessage& rtmp_msg); // 处理RTMP消息
    bool HandleInvoke(RtmpMessage& rtmp_msg); // 处理命令调用消息

    // RTMP连接状态处理
    bool Handshake();
    bool Connect();
    bool CretaeStream();
    bool Publish();
    bool DeleteStream();

    bool HandleResult(RtmpMessage& rtmp_msg); // 处理命令执行结果
    bool HandleOnStatus(RtmpMessage& rtmp_msg); // 处理状态通知
    void SetChunkSize(); // 设置块大小

    // 发送命令调用和通知消息
    bool SendInvokeMsg(uint32_t csid, std::shared_ptr<char> playload, uint32_t playload_size);
    bool SendNotifyMsg(uint32_t csid, std::shared_ptr<char> playload, uint32_t playload_size);
    // 检查视频数据是否为关键帧
    bool IsKeyFrame(std::shared_ptr<char> data, uint32_t size);
    // 发送视频和音频数据，以及RTMP块数据
    bool SendVideoData(uint64_t timestamp, std::shared_ptr<char> playload, uint32_t playload_size);
    bool SendAudioData(uint64_t timestamp, std::shared_ptr<char> playload, uint32_t playload_size);
    void SendRtmpChunks(uint32_t csid, RtmpMessage& rtmp_msg);

private:
    // 私有构造函数
    RtmpConnection(TaskScheduler *scheduler, int sockfd, Rtmp* rtmp);
    std::string app_;
    std::string stream_name_;
    std::string stream_path_;

    uint32_t number_ = 0;
    uint32_t stream_id_ = 0;
    uint32_t max_chunk_size_ = 128;
    uint32_t avc_sequence_header_size_ = 0;
    uint32_t aac_sequence_header_size_ = 0;

    bool is_publishing_ = false;
    bool has_key_frame_ = false;

    ConnectionState state_;
    TaskScheduler *task_scheduler_;

    std::weak_ptr<RtmpPublisher> rtmp_publisher_;

    std::unique_ptr<AmfDecoder> amf_decoder_ = nullptr;
    std::unique_ptr<AmfEncoder> amf_encoder_ = nullptr;
    std::unique_ptr<RtmpChunk> rtmp_chunk_ = nullptr;
    std::unique_ptr<RtmpHandshake> handshake_ = nullptr;

    std::shared_ptr<char> avc_sequence_header_ = nullptr;
    std::shared_ptr<char> aac_sequence_header_ = nullptr;
};
#endif
