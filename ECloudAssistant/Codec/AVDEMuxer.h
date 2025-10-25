#ifndef AVDEMUXER_H
#define AVDEMUXER_H
#include "AV_Common.h"
#include <functional>
#include <thread>

class AAC_Decoder;
class H264_Decoder;
// 音视频解复用器，能够从媒体文件中提取音频和视频流，并将它们分别传递给对应的解码器进行解码
class AVDEMuxer
{
public:
    AVDEMuxer(AVContext* ac);
    ~AVDEMuxer();
    bool Open(const std::string& path);    // 打开指定路径的媒体文件并开始解复用
    using StreamCallBack = std::function<void(bool)>;   // 设置回调函数
    // 回调函数用于通知用户流信息是否获取成功
    inline void SetStreamCallBack(const StreamCallBack& cb){streamCb_ = cb;}
protected:
    void Close();
    void FetchStream(const std::string& path);      // 从媒体文件中读取数据包并分发给对应的解码器
    bool FetchStreamInfo(const std::string& path);  // 获取媒体文件的流信息
    // 获取音频和视频的时长
    double audioDuration();
    double videoDuration();
    // 作为回调函数，用于检查是否需要中断解复用过程
    static int InterruptFouction(void* arg);
private:
    // 存储视频流和音频流的索引
    int videoIndex = -1;
    int audioIndex = -1;
    AVContext* avContext_;  // 存储解码器的配置信息
    AVDictionary* avDict_;  // 存储媒体文件的选项
    std::atomic_bool quit_ = false; // 原子布尔变量，用于控制解复用线程的退出
    StreamCallBack streamCb_ = [](bool){};  // 流回调函数，当流信息获取完成时调用
    AVFormatContext* pFormateCtx_ = nullptr;    // 用于处理媒体文件的格式和流信息
    std::unique_ptr<std::thread> readthread_;   // 指向解复用线程的智能指针
    //解码器，用于处理提取出来的流数据
    std::unique_ptr<AAC_Decoder> aacDecoder_;
    std::unique_ptr<H264_Decoder> h264Decoder_;
};

#endif // AVDEMUXER_H
