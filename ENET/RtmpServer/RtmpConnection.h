#include "../EdoyunNet/TcpConnection.h"
#include "amf.h"
#include "rtmp.h"
#include "RtmpSink.h"
#include "RtmpChunk.h"
#include "RtmpHandshake.h"

class RtmpServer;
class RtmpSession;
// 负责处理RTMP协议的握手过程、消息解析、媒体数据发送等操作
class RtmpConnection : public TcpConnection , public RtmpSink
{
public:
    // 连接的状态
    enum ConnectionState
    {
        HANDSHAKE, // 握手阶段
        START_CONNECT, // 开始连接
        START_CREATE_STREAM, // 开始创建流
        START_DELETE_STREAM, // 开始删除流
        START_PLAY, // 开始播放
        START_PUBLISH, // 开始推流
    };
    RtmpConnection(std::shared_ptr<RtmpServer> rtmp_server,TaskScheduler* scheduler,int socket);
    virtual ~RtmpConnection();
    // 状态检查函数
    virtual bool IsPlayer() override{return state_ == START_PLAY;}
    virtual bool IsPublisher() override{return state_ == START_PUBLISH;}
    virtual bool IsPlaying() override{return is_playing_;}
    virtual bool IsPublishing()override{return is_publishing_;}
    virtual uint32_t GetId() override{return this->GetSocket();};
private:
    RtmpConnection(TaskScheduler* scheduler,int socket,Rtmp* rtmp);
    // 从网络读取的数据
    bool OnRead(BufferReader& buffer);
    void OnClose();
    // 处理RTMP块数据
    bool HandleChunk(BufferReader& buffer);
    // 处理RTMP消息
    bool HandleMessage(RtmpMessage& rtmp_msg);
    // 处理命令调用消息
    bool HandleInvoke(RtmpMessage& rtmp_msg);
    // 处理通知消息
    bool HandleNotify(RtmpMessage& rtmp_msg);
    // 处理音频数据
    bool HandleAudio(RtmpMessage& rtmp_msg);
    // 处理视频数据
    bool HandleVideo(RtmpMessage& rtmp_msg);

    bool HandleConnect();   // 处理客户端连接请求
    bool HandleCreateStream();  // 处理创建流请求
    bool HandlePublish();       // 处理推流请求
    bool HandlePlay();          // 处理播放请求
    bool HandleDeleteStream();  // 处理删除流请求

    // 设置对端带宽
    void SetPeerBandWidth();
    // 发送确认消息
    void SendAcknowlegement();
    // 设置块大小
    void SetChunkSize();
    
    // 发送命令调用信息
    bool SendInvokeMessage(uint32_t csid,std::shared_ptr<char> playload,uint32_t playload_size);
    // 发送通知消息
    bool SendNotifyMessage(uint32_t csid,std::shared_ptr<char> playload,uint32_t playload_size);
    
    // 检查视频数据是否为关键帧 发送RTMP块数据
    bool IsKeyFrame(std::shared_ptr<char> data,uint32_t size);
    void SendRtmpChunks(uint32_t csid,RtmpMessage& rtmp_msg);

    virtual bool SendMetaData(AmfObjects metaData)override; //发送消息，通知后面的数据为元数据
    virtual bool SendMediaData(uint8_t type,uint64_t timestamp,std::shared_ptr<char> playload,uint32_t playload_size)override; //纯虚函数
private:
    ConnectionState state_; // 当前连接的状态
    std::shared_ptr<RtmpHandshake> handshake_;  // 管理RTMP握手过程
    std::shared_ptr<RtmpChunk> rtmp_chunk_;     // 处理RTMP块数据

    std::weak_ptr<RtmpServer> rtmp_server_;     // 指向所属的RTMP服务器
    std::weak_ptr<RtmpSession> rtmp_session_;   // 指向当前会话

    uint32_t peer_width_ = 5000000;
    uint32_t ackonwledgement_size_ = 5000000;
    uint32_t max_chunk_size_ = 128;
    uint32_t stream_id_ = 0;

    AmfObjects meta_data_;      
    AmfDecoder amf_decoder_;    // AMF数据的解码器
    AmfEncoder amf_encoder_;    // AMF数据的编码器

    bool is_playing_ = false;
    bool is_publishing_ = false;

    std::string app_;           // 应用程序名词
    std::string stream_name_;   // 流名称
    std::string stream_path_;   // 流路径

    bool has_key_frame = false;

    //元数据
    // 视频和音频的序列头数据，以及大小
    std::shared_ptr<char> avc_sequence_header_; // 存储Avc(H.264)适配的序列数据 是解码媒体流的关键信息
    std::shared_ptr<char> aac_sequence_header_; // 存储AAC音频的序列头数据
    uint32_t avc_sequence_header_size_ = 0;
    uint32_t aac_sequence_header_szie_ = 0;


};