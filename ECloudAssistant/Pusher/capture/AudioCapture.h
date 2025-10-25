#ifndef AUDIOCAPTURE_H
#define AUDIOCAPTURE_H
#include <thread>
#include <cstdint>
#include <memory>

class AudioBuffer;
class WASAPICapture;

// 使用WASAPI进行音频输入，并通过一个音频缓冲区来管理捕获到的音频数据
class AudioCapture
{
public:
    AudioCapture();
    ~AudioCapture();
public:
    bool Init(uint32_t size = 20480); // 指定音频缓冲区大小，为2M
    void Close();
    int GetSamples();   // 获取缓冲区当前可用的音频样本数量
    // 从缓冲区读取指定数量的音频样本到目标缓冲区
    int Read(uint8_t* data,uint32_t samples); 
    // 音频捕获是否已启动
    inline bool CaptureStarted()const{return is_stared_;}
    // 捕获音频相关函数
    inline uint32_t GetChannels()const {return channels_;}
    inline uint32_t GetSamplerate()const {return samplerate_;}
    inline uint32_t GetBitsPerSample()const {return bits_per_sample_;}
private:
    int StartCapture();
    int StopCapture();
    bool is_initailed_ = false;
    bool is_stared_ = false;
    uint32_t channels_ = 2;     // 通道数
    uint32_t samplerate_ = 48000;   // 采样率 48khz
    uint32_t bits_per_sample_ = 16; // 位深度
    std::unique_ptr<WASAPICapture> capture_;   // 音频捕获指针
    std::unique_ptr<AudioBuffer> audio_buffer_; // 音频缓冲区
};

#endif // AUDIOCAPTURE_H
