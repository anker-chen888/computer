#include "AudioEncoder.h"
#include "Audio_Resampler.h"



AudioEncoder::AudioEncoder()
    :audio_resampler_(nullptr)
{

}

AudioEncoder::~AudioEncoder()
{
    Close();
}

// 初始化音频编码器和重采样器
bool AudioEncoder::Open(AVConfig &config)
{
    //初始化编码器
    if(is_initialzed_)
    {
        return false;
    }

    config_ = config;
    //创建编码器
    codec_ = const_cast<AVCodec*>(avcodec_find_encoder(AV_CODEC_ID_AAC));
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

    //设置编码器上下文参数
    codecContext_->sample_rate = config.audio.samplerate; // 采样率
    codecContext_->sample_fmt = AV_SAMPLE_FMT_FLTP;       // 采样格式
    codecContext_->channels = config.audio.channels;      // 声道数
    codecContext_->channel_layout = av_get_default_channel_layout(config.audio.channels); // 设置声道布局
    codecContext_->bit_rate = config.audio.bitrate;       // 设置比特率
    //获取全局的AAC头部
    codecContext_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    //打开编码器
    if(avcodec_open2(codecContext_,codec_,NULL) != 0)
    {
        Close();
        return false;
    }

    //创建重采样对象
    audio_resampler_.reset(new AudioResampler());
    //初始化这个重采样对象 将格式转为AV_SAMPLE_FMT_FLTP 32位单精度浮点数
    if(!audio_resampler_->Open(config.audio.samplerate,
                                config.audio.channels,
                                config.audio.format,
                                config.audio.samplerate,
                                config.audio.channels,
                                AV_SAMPLE_FMT_FLTP))
    {
        Close();
        return false;
    }

    is_initialzed_ = true;
    return true;
}

void AudioEncoder::Close()
{
    if(audio_resampler_)
    {
        audio_resampler_->Close();
        audio_resampler_.reset();
        audio_resampler_ = nullptr;
    }
}

// 获取帧样本数
uint32_t AudioEncoder::GetFrameSamples()
{
    //获取帧数
    if(is_initialzed_)
    {
        return codecContext_->frame_size;
    }

    return 0;
}

// 将PCM音频数据编码为AAC格式
AVPacketPtr AudioEncoder::Encode(const uint8_t *pcm, int samples)
{
    AVFramePtr in_frame(av_frame_alloc(),[](AVFrame* ptr){av_frame_free(&ptr);});
    //初始化这个输入帧
    in_frame->sample_rate = codecContext_->sample_rate;
    in_frame->format = AV_SAMPLE_FMT_FLT;
    in_frame->channels = codecContext_->channels;
    in_frame->channel_layout = codecContext_->channel_layout;
    in_frame->nb_samples = samples;
    in_frame->pts = pts_;
    in_frame->pts = av_rescale_q(pts_,{1,codecContext_->sample_rate},codecContext_->time_base);
    pts_ += in_frame->nb_samples;

    //创建内存
    if(av_frame_get_buffer(in_frame.get(),0) < 0)
    {
        return nullptr;
    }

    //计算位数
    int bytes_per_samples = av_get_bytes_per_sample(config_.audio.format);
    if(bytes_per_samples == 0)
    {
        return nullptr;
    }

    //接下来开始拷贝内存 pcm->in_frame
    memcpy(in_frame->data[0], pcm, bytes_per_samples * in_frame->channels * samples);

    //再开始重采样
    AVFramePtr fltp_frame = nullptr;
    if(audio_resampler_->Convert(in_frame, fltp_frame) <= 0)
    {
        return nullptr;
    }

    // 进行数据的编码
    int ret = avcodec_send_frame(codecContext_, fltp_frame.get());
    if(ret != 0)
    {
        return nullptr;
    }

    AVPacketPtr av_packet(av_packet_alloc(),[](AVPacket* ptr){av_packet_free(&ptr);});
    //初始化这个packet
    av_init_packet(av_packet.get());
    //开始接收packet 从编码器接受编码后的帧数据
    ret = avcodec_receive_packet(codecContext_, av_packet.get());
    if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
    {
        return nullptr;
    }
    else if(ret < 0)
    {
        return nullptr;
    }
    //接收成功
    return av_packet;
}
