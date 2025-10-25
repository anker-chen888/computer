#ifndef RTMPPUBLISHER_H
#define RTMPPUBLISHER_H
#include "rtmp.h"
#include "EventLoop.h"
#include "TimeStamp.h"
#include "RtmpConnection.h"

// 使用RTMP协议推送音频和视频流到RTMP流媒体服务器
class RtmpPublisher : public Rtmp, public std::enable_shared_from_this<RtmpPublisher>
{
public:
    static std::shared_ptr<RtmpPublisher> Create(EventLoop* loop);
    ~RtmpPublisher();
    // 设置媒体信息，如视频和音频的编码格式、分辨率等
    int  SetMediaInfo(MediaInfo media_info);
    // 打开指定的url，建立TCP连接
    int  OpenUrl(std::string url, int msec);
    // 推送视频帧数据到服务器
    int  PushVideoFrame(uint8_t *data, uint32_t size);
    // 推送音频数据到服务器
    int  PushAudioFrame(uint8_t *data, uint32_t size);
    void Close();           // 关闭连接
    bool IsConnected();     // 检查连接状态
private:
    RtmpPublisher(EventLoop *event_loop);
    // 判断是否为关键帧
    bool IsKeyFrame(uint8_t* data, uint32_t size);
    EventLoop *event_loop_ = nullptr;               // 事件循环
    TaskScheduler  *task_scheduler_ = nullptr;      // 任务调度器
    std::shared_ptr<RtmpConnection> rtmp_conn_;     // RTMP连接
    MediaInfo media_info_;                          // 媒体信息
    bool has_key_frame_ = false;                    // 是否有关键帧
    Timestamp timestamp_;                           // 时间戳
    std::shared_ptr<char> avc_sequence_header_;     // 视频序列头
    std::shared_ptr<char> aac_sequence_header_;     // 音频序列头
    uint32_t avc_sequence_header_size_ = 0;         // 视频序列头大小
    uint32_t aac_sequence_header_size_ = 0;         // 音频序列头大小
};
#endif // RTMPPUBLISHER_H
