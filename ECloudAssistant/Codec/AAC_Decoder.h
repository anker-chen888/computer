#ifndef AAC_DECODER_H
#define AAC_DECODER_H
#include <QThread>
#include "AV_Common.h"

class AudioResampler;
// 实现了AAC解码的功能，用于解码AAC音频流
class AAC_Decoder : public QThread ,public DecodBase
{
    Q_OBJECT
public:
    AAC_Decoder(AVContext* ac,QObject* parent = nullptr);
    AAC_Decoder(const AAC_Decoder&) = delete;
    AAC_Decoder& operator=(const AAC_Decoder&) = delete; // 防止对象被意外拷贝，确保资源管理的安全性
    ~AAC_Decoder();
    int  Open(const AVCodecParameters* codecParamer);
    inline void put_packet(const AVPacketPtr packet){audio_queue_.push(packet);}
    inline bool isFull(){return audio_queue_.size() > 50;} // 判断队列是否已经满了
protected:
    void Close();
    virtual void run()override;
private:
    bool            quit_ = false;          // 控制线程退出的标志
    AVQueue<AVPacketPtr>   audio_queue_;    // 存储音视频数据包的队列
    AVContext*      avContext_ = nullptr;   // 存储音频解码的上下文信息
    std::unique_ptr<AudioResampler> audioResampler_;    // 音频重采样器，用于音频数据的格式转换
};
#endif // AAC_DECODER_H
