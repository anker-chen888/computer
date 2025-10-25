#ifndef RTMPPUSHMANAGER_H
#define RTMPPUSHMANAGER_H
#include <thread>
#include <memory>
#include "RtmpPublisher.h"
#include "H264Encoder.h"
#include <QObject>

class AACEncoder;
class AudioCapture;
class GDIScreenCapture;

// 用于管理RTMP推流的整个过程，包括音频和视频的捕获、编码以及推流
// 是整个推流器的核心类，负责协调视频采集
class RtmpPushManager : public QObject
{
    Q_OBJECT
public:
    virtual ~RtmpPushManager();
    RtmpPushManager();
public:
    bool Open(const QString& str);  // 打开推流连接
    bool isClose(){return isConnect == false;}  // 是否连接
protected:
    bool Init();            // 初始化
    void Close();
    void EncodeVideo();     // 停止视频编码
    void EncodeAudio();     // 停止音频编码
    void StopEncoder();     // 停止编码
    void StopCapture();     // 停止捕获
    bool IsKeyFrame(const uint8_t* data, uint32_t size);    // 判断给定的数据是否是一个关键帧
    void PushVideo(const quint8* data, quint32 size);       // 发送视频数据
    void PushAudio(const quint8* data, quint32 size);       // 发送音频数据
private:
    bool exit_ = false;
    bool isConnect = false;
    EventLoop* loop_ = nullptr;
    std::unique_ptr<AACEncoder>  aac_encoder_;              // AAC编码
    std::unique_ptr<H264Encoder> h264_encoder_;             // H.264编码
    std::shared_ptr<RtmpPublisher> pusher_;                 // 实行音视频数据的发送
    std::unique_ptr<AudioCapture> audio_Capture_;           // 音频采集块
    std::unique_ptr<GDIScreenCapture> screen_Capture_;      // 视频采集块
    std::unique_ptr<std::thread>  audioCaptureThread_ = nullptr;    // 音频线程
    std::unique_ptr<std::thread>  videoCaptureThread_ = nullptr;    // 视频线程
};

#endif // RTMPPUSHMANAGER_H
