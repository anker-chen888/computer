﻿#ifndef H264_ENCODER_H
#define H264_ENCODER_H
#include <QtGlobal>
#include <vector>
#include "AV_Common.h"
class VideoEncoder;

// 将输入的RGBA格式图像数据编码为H.264视频流
class H264Encoder
{
public:
    H264Encoder();
    H264Encoder(const H264Encoder&) = delete;
    H264Encoder& operator=(const H264Encoder&) = delete;
    ~H264Encoder();
public:
    bool OPen(qint32 width,qint32 height,qint32 framerate,qint32 bitrate,qint32 format);
    void Close();
    qint32 Encode(quint8* rgba_buffer,quint32 width,quint32 height,quint32 size,std::vector<quint8>& out_frame);
    // 获取序列参数(SPS和PPS)，用于解码H.264视频流
    qint32 GetSequenceParams(quint8* out_buffer, qint32 out_buffer_size);
private:
    bool IsKeyFrame(AVPacketPtr pkt);
private:
    AVConfig config_;
    std::unique_ptr<VideoEncoder> h264_encoder_;
};
#endif // H264_ENCODER_H
