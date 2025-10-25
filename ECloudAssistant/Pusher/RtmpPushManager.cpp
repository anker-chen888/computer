﻿#include "RtmpPushManager.h"
#include "GDISreenScapture.h"
#include "AAC_Encoder.h"
#include "H264Encoder.h"
#include "AudioCapture.h"
#include "H264Paraser.h" // 解析H264文件--->NALU

RtmpPushManager::~RtmpPushManager()
{
    Close();
}

RtmpPushManager::RtmpPushManager()
    :aac_encoder_(nullptr)
    ,h264_encoder_(nullptr)
    ,pusher_(nullptr)
    ,audio_Capture_(nullptr)
    ,screen_Capture_(nullptr)
{
    loop_ = new EventLoop(1);
}

// 打开推流连接
bool RtmpPushManager::Open(const QString &str)
{
    if(!Init())
    {
        return false;
    }
    //通过推流器打开这个url
    if(pusher_->OpenUrl(str.toStdString(),1000) < 0) //解析url失败
    {
        return false;
    }

    isConnect = true;

    //开始采集视频 启动线程
    videoCaptureThread_.reset(new std::thread([this](){
        this->EncodeVideo();
    }));
    //开始采集音频  启动线程
    audioCaptureThread_.reset(new std::thread([this](){
        this->EncodeAudio();
    }));
    return true;
}

// 初始化组件
bool RtmpPushManager::Init()
{
    //创建一个推流器
    pusher_ = RtmpPublisher::Create(loop_);
    //设置块大小
    pusher_->SetChunkSize(60000);

    //创建视频采集
    screen_Capture_.reset(new GDIScreenCapture()); //采集器采集的像素要跟这个编码器初始化一致
    if(!screen_Capture_->Init())
    {
        return false;
    }

    //视频编码
    h264_encoder_.reset(new H264Encoder());
    if(!h264_encoder_->OPen(2560,1440,30,80000,AV_PIX_FMT_BGRA))//这个分辨率大家要根据自己电脑来设置
    {
        return false;
    }
    //音频采集
    audio_Capture_.reset(new AudioCapture());
    if(!audio_Capture_->Init())
    {
        return false;
    }
    //音频编码
    aac_encoder_.reset(new AACEncoder());
    //初始化
    if(!aac_encoder_->Open(audio_Capture_->GetSamplerate(),audio_Capture_->GetChannels(),AV_SAMPLE_FMT_S16,64))
    {
        return false;
    }
    //获取音频视频编码参数
    MediaInfo mediaInfo;
    uint8_t extradata[1024] = {0};
    int extradatdSize = 0;

    //获取H264编码参数
    extradatdSize = h264_encoder_->GetSequenceParams(extradata,1024);
    if(extradatdSize <= 0)
    {
        return false;
    }
    //获取sps pps 将其存储到MediaInfo结构中，以便后续的视频推流或解码过程中使用 也是H.264数据的头结构
    H264Paraser::Nal sps = H264Paraser::findNal(extradata,extradatdSize);
    if(sps.first != nullptr && sps.second != nullptr && (*sps.first & 0x1f) == 7)//sps数据
    {
        mediaInfo.sps_size = sps.second - sps.first + 1;
        mediaInfo.sps.reset(new uint8_t[mediaInfo.sps_size],std::default_delete<uint8_t[]>());
        memcpy(mediaInfo.sps.get(),sps.first,mediaInfo.sps_size);
        //pps
        H264Paraser::Nal pps = H264Paraser::findNal(sps.second,extradatdSize - (sps.second - (uint8_t*)extradata));
        if(pps.first != nullptr && pps.second != nullptr && (*pps.first & 0x1f) == 8)//pps数据
        {
            mediaInfo.pps_size = pps.second - pps.first + 1;
            mediaInfo.pps.reset(new uint8_t[mediaInfo.pps_size],std::default_delete<uint8_t[]>());
            memcpy(mediaInfo.pps.get(),pps.first,mediaInfo.pps_size);
        }
    }
    //添加音频参数
    uint32_t audioExtraSize = aac_encoder_->GetSpecificConfig(extradata,1024);

    mediaInfo.audio_specific_config_size = audioExtraSize;
    mediaInfo.audio_specific_config.reset(new uint8_t[mediaInfo.audio_specific_config_size],std::default_delete<uint8_t[]>());
    //拷贝数据
    memcpy(mediaInfo.audio_specific_config.get(),extradata,audioExtraSize);

    //发送这个编码参数
    pusher_->SetMediaInfo(mediaInfo);
    return true;
}

void RtmpPushManager::Close()
{
    //释放资源
    exit_ = true;//结束线程
    isConnect = false;
    if(pusher_ && pusher_->IsConnected())
    {
        pusher_->Close();
        pusher_.reset();
        pusher_ = nullptr;
    }

    StopEncoder();
    StopCapture();

}
// 视频编码并发送
void RtmpPushManager::EncodeVideo()
{
    //我们控制发送速率 每一秒发送30张
    static Timestamp timeStamp;
    uint32_t fameRate = 30;
    while(!exit_ && isConnect)
    {
        uint32_t elapsed = timeStamp.Elapsed();
        //获取延迟时间
        uint32_t delay = fameRate;
        if(elapsed > delay) // 间隔时间大于延迟
        {
            //重置延迟
            delay = 0;
        }
        else
        {
            delay -= elapsed;
        }
        //休眠这个延迟时间
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        //重新获取这个时间
        timeStamp.Reset();
        FrameContainer bgra_image;
        uint32_t width = 0,height = 0;
        //采集
        if(screen_Capture_ && h264_encoder_ && pusher_)
        {
            if(screen_Capture_->CaptureFrame(bgra_image,width,height))
            {
                //采集之后，我们需要编码
                FrameContainer out_frame;
                if(h264_encoder_->Encode(&bgra_image[0],width,height,bgra_image.size(),out_frame) > 0)
                {
                    //编码之后开始推送
                    if(out_frame.size() > 0)
                    {
                        PushVideo(&out_frame[0],out_frame.size());
                    }
                }
            }
        }
    }
}
// 音频编码并发送
void RtmpPushManager::EncodeAudio()
{
    //准备buffer存放音频数据
    std::shared_ptr<uint8_t> pcm_buffer(new uint8_t[48000 * 8],std::default_delete<uint8_t[]>());
    //获取样本数
    uint32_t frame_samples = aac_encoder_->GetFrames();
    while(!exit_ && isConnect)
    {
        if(audio_Capture_->GetSamples() >= (int)frame_samples)
        {
            if(audio_Capture_->Read(pcm_buffer.get(),frame_samples) != frame_samples)
            {
                continue;//数据不全
            }
            //编码aac
            AVPacketPtr pkt_ptr = aac_encoder_->Encode(pcm_buffer.get(), frame_samples);
            if(pkt_ptr)
            {
                //推送
                PushAudio(pkt_ptr->data,pkt_ptr->size);
            }
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1)); // 休眠
        }
    }
}


void RtmpPushManager::StopEncoder()
{
    if(audioCaptureThread_)
    {
        //结束线程
        audioCaptureThread_->join();
        audioCaptureThread_.reset();
        audioCaptureThread_ = nullptr;
    }

    if(videoCaptureThread_)
    {
        //结束线程
        videoCaptureThread_->join();
        videoCaptureThread_.reset();
        videoCaptureThread_ = nullptr;
    }

    //编码器
    if(h264_encoder_)
    {
        h264_encoder_->Close();
        h264_encoder_.reset();
        h264_encoder_ = nullptr;
    }

    if(aac_encoder_)
    {
        aac_encoder_->Close();
        aac_encoder_.reset();
        aac_encoder_ = nullptr;
    }
}

void RtmpPushManager::StopCapture()
{
    if(audio_Capture_)
    {
        audio_Capture_->Close();
        audio_Capture_.reset();
        audio_Capture_ = nullptr;
    }

    if(screen_Capture_)
    {
        screen_Capture_->Close();
        screen_Capture_.reset();
        screen_Capture_ = nullptr;
    }
}

// 判断是否为关键帧
bool RtmpPushManager::IsKeyFrame(const uint8_t *data, uint32_t size)
{
    //判断关键帧 startcode 3 4
    int startcode = 0;
    if(data[0] == 0 && data[1] == 0 && data[2] == 0)
    {
        startcode = 3;
    }
    else if(data[0] == 0 && data[1] == 0 && data[2] == 0 && data[3] == 0)
    {
        startcode = 4;
    }

    //再去获取类型
    int type = data[startcode] & 0x1f;
    if(type == 5 || type == 7)//关键帧
    {
        return true;
    }
    return false;
}

// 发送视频
void RtmpPushManager::PushVideo(const quint8 *data, quint32 size)
{
    //推送视频
    //准备buffer size = video size - 4 //startcode
    std::shared_ptr<uint8_t> frame(new uint8_t[size - 4],std::default_delete<uint8_t[]>());
    //拷贝数据
    memcpy(frame.get(),data + 4,size - 4);
    if(size > 0)
    {
        if(pusher_ && pusher_->IsConnected())
        {
            pusher_->PushVideoFrame(frame.get(),size - 4);
        }
    }

}

// 发送音频
void RtmpPushManager::PushAudio(const quint8 *data, quint32 size)
{
    std::shared_ptr<uint8_t> frame(new uint8_t[size],std::default_delete<uint8_t[]>());
    //拷贝数据
    memcpy(frame.get(),data,size);
    if(size > 0)
    {
        if(pusher_ && pusher_->IsConnected())
        {
            pusher_->PushAudioFrame(frame.get(),size);
        }
    }
}
