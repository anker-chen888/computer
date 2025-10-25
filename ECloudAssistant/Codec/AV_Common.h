#ifndef AV_COMMOEN_H
#define AV_COMMOEN_H
#include <QtGlobal>
#include <QDebug>
#include <memory>
#include <mutex>
#include "AV_Queue.h"
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include "libavutil/error.h"
}

using AVPacketPtr = std::shared_ptr<AVPacket>;
using AVFramePtr  = std::shared_ptr<AVFrame>;

// 视频的配置参数
typedef struct VIDEOCONFIG
{
    quint32 width; // 分辨率 宽
    quint32 height; // 分辨率 高
    quint32 bitrate; // 比特率
    quint32 framerate; // 帧率
    quint32 gop; // GOP大小
    AVPixelFormat format; // 像素格式
}VideoConfig;

// 音频的配置参数
typedef struct AUDIOCONFIG
{
    quint32 channels;   // 声道数
    quint32 samplerate; // 采样率
    quint32 bitrate;    // 比特率
    AVSampleFormat format; // 样本格式
}AudioConfig;

// 音视频参数配置
struct AVConfig
{
    VideoConfig video;
    AudioConfig audio;
};

struct AVContext
{
public:
    //音频相关参数//
    int32_t audio_sample_rate;
    int32_t audio_channels_layout;
    AVRational audio_src_timebase;
    AVRational audio_dst_timebase;
    AVSampleFormat audio_fmt;
    double audioDuration;
    AVQueue<AVFramePtr> audio_queue_;   // 音频帧队列
    //视频相关参数
    int32_t video_width;
    int32_t video_height;
    AVRational video_src_timebase;
    AVRational video_dst_timebase;
    AVPixelFormat video_fmt;
    double videoDuration;
    AVQueue<AVFramePtr> video_queue_; // 视频帧队列

    int avMediatype_ = 0;
};

// 编码器基类
class EncodBase
{
public:
    EncodBase():is_initialzed_(false),codec_(nullptr),codecContext_(nullptr){config_ = {};}
    virtual ~EncodBase(){if(codecContext_)avcodec_free_context(&codecContext_);}
    EncodBase(const EncodBase&) = delete;
    EncodBase& operator=(const EncodBase&) = delete;
public:
    virtual bool Open(AVConfig& config) = 0;
    virtual void Close() = 0;
    AVCodecContext* GetAVCodecContext() const // 获取编码器的上下文
    {return codecContext_;}
protected:
    bool is_initialzed_ = false;
    AVConfig config_;
    AVCodec* codec_;
    AVCodecContext *codecContext_ = nullptr;
};

// 解码器基类
class DecodBase
{
public:
    DecodBase():is_initial_(false),video_index_(-1),audio_index_(-1),codec_(nullptr),codecCtx_(nullptr){config_ = {};}
    virtual ~DecodBase(){if(codecCtx_){avcodec_free_context(&codecCtx_);};}
    DecodBase(const DecodBase&) = delete;
    DecodBase& operator=(const DecodBase&) = delete;
    AVCodecContext* GetAVCodecContext() const
    {return codecCtx_;}
protected:
    bool is_initial_; // 是否初始化
    std::mutex mutex_;
    qint32 video_index_; // 视频流的索引
    qint32 audio_index_;    // 音频流的索引
    AVConfig config_;   // 解码器的配置参数，包含视频和音频的详细设置
    AVCodec* codec_;    // 表示使用的解码器
    AVCodecContext *codecCtx_;  // 包含解码器的上下文信息
};

#endif //AV_COMMOEN_H
