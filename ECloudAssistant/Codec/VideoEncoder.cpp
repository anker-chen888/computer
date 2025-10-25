﻿#include "VideoEncoder.h"
#include "VideoConvert.h"

extern "C"
{
    #include <libavutil/rational.h>
    #include <libavutil/error.h>
    #include<libavutil/opt.h>
}

VideoEncoder::VideoEncoder()
    :pts_(0)
    ,width_(0)
    ,height_(0)
    ,force_idr_(false)
    ,rgba_frame_(nullptr)
    ,h264_packet_(nullptr)
    ,converter_(nullptr)
{
    //创建AVframe avpacket
    rgba_frame_.reset(av_frame_alloc(),[](AVFrame* ptr){av_frame_free(&ptr);});
    h264_packet_.reset(av_packet_alloc(),[](AVPacket* ptr){av_packet_free(&ptr);});
}

VideoEncoder::~VideoEncoder()
{
    Close();
}

bool VideoEncoder::Open(AVConfig &video_config)
{
    //初始化
    if(is_initialzed_)
    {
        Close();
    }

    config_ = video_config;

    //查找编码器H.264
    codec_ = const_cast<AVCodec*>(avcodec_find_encoder(AV_CODEC_ID_H264));
    if(!codec_)
    {
        Close();
        return false;
    }

    //创建编码器上下文
    codecContext_ = avcodec_alloc_context3(codec_);
    if(!codecContext_)
    {
        Close();
        return false;
    }

    //配置上下文参数
    codecContext_->width = config_.video.width;
    codecContext_->height = config_.video.height;
    codecContext_->time_base = {1,(qint32)config_.video.framerate};//帧率倒数
    codecContext_->framerate = {(qint32)config_.video.framerate,1};
    codecContext_->gop_size = 30;
    codecContext_->max_b_frames = 0;//降低延迟
    codecContext_->pix_fmt = AV_PIX_FMT_YUV420P;
    codecContext_->bit_rate = config_.video.bitrate;
    //还需要设置全局头
    codecContext_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    //还需要加上参数，降低编码延迟
    codecContext_->rc_min_rate = config_.video.bitrate;
    codecContext_->rc_max_rate = config_.video.bitrate;
    codecContext_->rc_buffer_size = config_.video.bitrate;
    //设置字典
    av_opt_set(codecContext_->priv_data,"tune","zerolatency",0);//极速编码
    av_opt_set(codecContext_->priv_data,"preset","ultrafast",0);//极速编码

    //打开编码器
    if(avcodec_open2(codecContext_,codec_,NULL) != 0)
    {
        Close();
        return false;
    }

    width_ = config_.video.width;
    height_ = config_.video.height;
    is_initialzed_ = true;
    return true;
}

void VideoEncoder::Close()
{
    width_ = 0;
    height_ = 0;
    pts_ = 0;
    is_initialzed_ = false;
    if(converter_)
    {
        converter_->Close();
        converter_.reset();
        converter_ = nullptr;
    }
}

AVPacketPtr VideoEncoder::Encode(const quint8 *data, quint32 width, quint32 height, quint32 data_size, quint64 pts)
{
    //开始编码
    if(!is_initialzed_)
    {
        return nullptr;
    }

    //此外我们需要去初始化转换器，如果这个输入宽高跟目标不一致我们就需要创建转换器
    if(width_ != width || height_ != height || !converter_)
    {
        converter_.reset(new VideoConverter());
        //初始化视频转换器
        if(!converter_->Open(width,height,(AVPixelFormat)config_.video.format,
                              codecContext_->width,codecContext_->height,codecContext_->pix_fmt))
        {
            //初始化失败
            converter_.reset();
            converter_ = nullptr;
            return nullptr;
        }

        rgba_frame_->width = width_;
        rgba_frame_->height = height_;
        //指定格式
        rgba_frame_->format = config_.video.format;
        //获取内存
        if(av_frame_get_buffer(rgba_frame_.get(),32) != 0)//四字节rgba
        {
            return nullptr;
        }
    }

    //我们将输入数据转到这个rgbaframe来去转换
    memcpy(rgba_frame_->data[0],data,data_size);

    //转换
    if(!converter_)
    {
        return nullptr;
    }

    //开始转换
    //准备输出
    AVFramePtr out_frame = nullptr;
    if(converter_->Convert(rgba_frame_,out_frame) <= 0)
    {
        return nullptr;
    }

    //更新out_frame参数
    out_frame->pts = pts >= 0 ? pts : pts_++;
    out_frame->pict_type = AV_PICTURE_TYPE_NONE;

    //再去编码
    if(avcodec_send_frame(codecContext_,out_frame.get()) < 0)
    {
        return nullptr;
    }
    //我们再去接收这个值
    int ret = avcodec_receive_packet(codecContext_,h264_packet_.get());
    if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
    {
        return nullptr;
    }
    else if(ret < 0)
    {
        return nullptr;
    }
    return h264_packet_;
}
