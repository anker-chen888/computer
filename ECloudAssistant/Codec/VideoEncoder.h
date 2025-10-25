#ifndef VIDEO_ENCODER_H
#define VIDEO_ENCODER_H
#include <memory>
#include "AV_Common.h"
#include "VideoConvert.h"
class VideoEncoder : public EncodBase
{
public:
    VideoEncoder();
    VideoEncoder(const VideoEncoder&) = delete;
    VideoEncoder& operator=(const VideoEncoder&) = delete;
    ~VideoEncoder();
public:
    virtual bool Open(AVConfig& video_config) override;
    virtual void Close()override;
    virtual AVPacketPtr Encode(const quint8* data,quint32 width,quint32 height,quint32 data_size,quint64 pts = 0);
private:
    qint64  pts_; // 存储当前帧的时间戳
    quint32 width_; // 存储视频的宽度和高度
    quint32 height_;
    bool force_idr_;    // 一个序列的第一帧IDR
    AVFramePtr  rgba_frame_; // 存储RGBA格式的视频帧数据
    AVPacketPtr h264_packet_;   // 存储编码后的H.264数据包
    std::unique_ptr<VideoConverter> converter_; // 用于视频格式转换
};
#endif // VIDEO_ENCODER_H
