#ifndef AUDIO_RENDER_H
#define AUDIO_RENDER_H
#include <QAudioOutput>
#include "AV_Common.h"

// 是一个音频播放器，用于将音频数据输出到音频设备
class AudioRender
{
public:
    AudioRender();
    ~AudioRender();
    inline bool IsInit(){return is_initail_;}
    //获取缓冲区pcm大小
    int AvailableBytes();
    bool InitAudio(int nChannels,int SampleRate,int nSampleSize);
    //播放音频
    void Write(AVFramePtr frame);
private:
    bool is_initail_ = false; // 标记音频渲染器是否已经初始化
    int  nSampleSize_ = -1;   // 音频样本的大小(位数)
    int  volume_ = 50;        // 输出音量
    
    QAudioFormat  audioFmt_;    // 音频格式设置
    QIODevice*    device_ = nullptr;    // 音频输出设备
    QAudioOutput* audioOut_ = nullptr;  // 音频输出对象
};

#endif
