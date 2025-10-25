#include "AVDEMuxer.h"
#include "H264_Decoder.h"
#include "AAC_Decoder.h"


AVDEMuxer::AVDEMuxer(AVContext *ac)
    :avContext_(ac)
    ,avDict_(nullptr)
    ,readthread_(nullptr)
{
    aacDecoder_.reset(new AAC_Decoder(avContext_));
    h264Decoder_.reset(new H264_Decoder(avContext_));
    //设置字典
//    av_dict_set(&avDict_,"stimeout","1000000",0);
//    av_dict_set(&avDict_,"analyzeduration","0",0);//设为0可以快速播放
//    av_dict_set(&avDict_,"fflags","nobuffer",0);//可以快速播放
//    av_dict_set(&avDict_,"flags","low_delay",0);//无延迟
//    av_dict_set(&avDict_,"tune","zerolatency",0);//零延迟

    av_dict_set(&avDict_, "stimeout", "1000000", 0);    // 设置超时时间
    av_dict_set(&avDict_, "analyzeduration", "0", 0);   // 设置分析持续事件
    av_dict_set(&avDict_, "max_analyze_duration","10",0);   // 设置最大分析时间
    av_dict_set(&avDict_, "fflags", "nobuffer", 0);     // 快速播放
    av_dict_set(&avDict_, "flags", "low_delay", 0);     // 无延迟
    av_dict_set(&avDict_, "tune","zerolatency",0);      // 零延迟
    av_dict_set(&avDict_, "reconnect", "0", 0);         // 设置重连接选项
    av_dict_set(&avDict_, "reconnect_streamed", "0", 0);    // 设置重新连接流选项
    av_dict_set(&avDict_, "reconnect_delay_max", "5", 0);   // 设置最大重新连接延迟

    //创建这个封装格式上下文
    pFormateCtx_ = avformat_alloc_context();
    //设置参数
    pFormateCtx_->max_ts_probe = 50;    // 设置最大事件戳探测值
    pFormateCtx_->probesize = 500000; // 设置探测大小
    pFormateCtx_->interrupt_callback.callback = InterruptFouction;//通过这个回调函数来去退出
    pFormateCtx_->interrupt_callback.opaque = this;     // 回调函数的上下文参数
    pFormateCtx_->flags |= AVFMT_FLAG_DISCARD_CORRUPT; // 丢器损坏的数据包，加速
}

AVDEMuxer::~AVDEMuxer()
{
    Close();
}

bool AVDEMuxer::Open(const std::string &path)
{
    //启动线程去查询流消息
    readthread_.reset(new std::thread([this,path](){
        this->FetchStream(path);
    }));
    return true;
}

void AVDEMuxer::Close()
{
    //将退出标志置为true;
    quit_ = true;
    //释放字典
    if(avDict_)
    {
        av_dict_free(&avDict_);
    }
    //回收线程资源
    if(readthread_)
    {
        if(readthread_->joinable())
        {
            readthread_->join();
            readthread_.reset();
            readthread_ = nullptr;
        }
    }
    //释放这个封装格式上下文
    if(pFormateCtx_)
    {
        avformat_close_input(&pFormateCtx_);
        pFormateCtx_ = nullptr;
    }

}

// 从媒体文件中读取数据包并分发个对应的解码器
void AVDEMuxer::FetchStream(const std::string &path)
{
    bool ret = true;
    //读流信息
    //返回这个流查询结果
    if(streamCb_)
    {
        ret = FetchStreamInfo(path);
        streamCb_(ret);
    }
    if(!ret)
    {
        return;
    }
    //如果这个结果为true就是有流信息，我们开始读取数据；
    AVPacketPtr packet = nullptr;
    while(!quit_ && pFormateCtx_)
    {
        packet = AVPacketPtr(av_packet_alloc(),[](AVPacket* p){av_packet_free(&p);});
        // 开始读取数据包，并分发到不同的队列中
        int r = av_read_frame(pFormateCtx_,packet.get());
        if(r == 0) //读取成功
        {
            if(packet->stream_index == videoIndex)//h264数据包
            {
                //将数据传到这个h.264解码器队列中
                h264Decoder_->put_packet(packet);
            }
            else if(packet->stream_index == audioIndex)
            {
                //将数据传到AAC解码器队列
                aacDecoder_->put_packet(packet);
            }
            else
            {
                //释放包
                av_packet_unref(packet.get());
            }
        }
        else
        {
            av_packet_unref(packet.get());
            break;
        }
    }

}

// 获取媒体文件的流信息，并初始化对应的解码器
bool AVDEMuxer::FetchStreamInfo(const std::string &path)
{
    // 使用avformat_open_input函数打开路径的媒体文件
    if(avformat_open_input(&pFormateCtx_,path.c_str(),nullptr,&avDict_) != 0)
    {
        return false;
    }
    // 使用avformat_find_stream_info获取媒体文件的流信息
    if(avformat_find_stream_info(pFormateCtx_,nullptr) < 0)//查询失败
    {
        return false;
    }
    //遍历媒体文件中的所有流，找到视频流和音频流的索引
    for(int i = 0;i < pFormateCtx_->nb_streams;i++)
    {   
        // 如果是视频流
        if(pFormateCtx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            //更新视频流索引
            videoIndex = i;
            //时长
            avContext_->videoDuration = pFormateCtx_->streams[i]->duration * av_q2d(pFormateCtx_->streams[i]->time_base);
        }
        // 如果是音频流
        else if(pFormateCtx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            //更新视频流索引
            audioIndex = i;
            //时长
            avContext_->audioDuration = pFormateCtx_->streams[i]->duration * av_q2d(pFormateCtx_->streams[i]->time_base);
        }
    }

    if(videoIndex != -1) //存在视频流
    {
        //初始化视频解码器
        if(h264Decoder_->Open(pFormateCtx_->streams[videoIndex]->codecpar) != 0)
        {
            h264Decoder_.reset();
            h264Decoder_ = nullptr;
            return false;
        }
    }
    if(audioIndex != -1) //存在音频
    {
        //初始化音频解码器
        if(aacDecoder_->Open(pFormateCtx_->streams[audioIndex]->codecpar) != 0)
        {
            aacDecoder_.reset();
            aacDecoder_ = nullptr;
            return false;
        }
    }
    return true;
}

// 音频持续时长
double AVDEMuxer::audioDuration()
{
    return avContext_->audioDuration;
}

// 视频持续时长
double AVDEMuxer::videoDuration()
{
    return avContext_->videoDuration;
}

int AVDEMuxer::InterruptFouction(void *arg)
{
    //退出标志
    AVDEMuxer* thiz = (AVDEMuxer*)arg;
    return thiz->quit_; //0 false 1 true;
}
