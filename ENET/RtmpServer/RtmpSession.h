#include <memory>
#include <mutex>
#include "amf.h"

class RtmpSink;
class RtmpConnection;

// RtmpSession类用于管理RTMP会话，处理音视频数据的发送、客户端的添加与移除等操作。
class RtmpSession
{
public:
    using Ptr = std::shared_ptr<RtmpSession>;
    RtmpSession();
    virtual ~RtmpSession();
    // 序列头数据包含了媒体的编解码参数，是解码器正确解析后续媒体数据的关键
    // 设置视频序列头
    void SetAvcSequenceHeader(std::shared_ptr<char> avcSequenceHeader,uint32_t avcSequenceHeaderSize)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        avc_sequence_header_ = avcSequenceHeader;
        avc_sequence_header_size_ = avcSequenceHeaderSize;
    }
    // 处理音频序列头
    void SetAacSequenceHeader(std::shared_ptr<char> aacSequenceHeader,uint32_t aacSequenceHeaderSize)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        aac_sequence_header_ = aacSequenceHeader;
        aac_sequence_header_size_ = aacSequenceHeaderSize;
    }

    //session添加客户端
    void AddSink(std::shared_ptr<RtmpSink> sink);
    //移除客户端
    void RemoveSink(std::shared_ptr<RtmpSink> sink);
    //获取当前会话中的客户端数量，包括主播和观众
    int GetClients();
    //发送元数据，如音视频编码
    void SendMetaData(AmfObjects& metaData);
    //发送媒体数据，如音视频帧
    void SendMediaData(uint8_t type,uint64_t timestamp,std::shared_ptr<char> data,uint32_t size);
    //获取推流对象
    std::shared_ptr<RtmpConnection> GetPublisher();
private:
    std::mutex mutex_;
    bool has_publisher_ = false; // 是否有推流客户端
    std::weak_ptr<RtmpSink> publisher_;
    // 存储会话中的所有客户端，键为客户端ID，值为客户端的弱引用
    std::unordered_map<int,std::weak_ptr<RtmpSink>> rtmp_sinks_;
    // 视频（AVC）和音频（AAC）的序列头数据。
    std::shared_ptr<char> avc_sequence_header_;
    std::shared_ptr<char> aac_sequence_header_;
    uint32_t avc_sequence_header_size_ = 0;
    uint32_t aac_sequence_header_size_ = 0;
};