#ifndef GDISCREENCAPTURE_H
#define GDISCREENCAPTURE_H
#include <QThread>
#include <memory>
#include <mutex>

// 声明了ffmpeg相关的结构体
struct AVFrame;     // 多媒体帧(原始的)
struct AVPacket;    // 多媒体数据包(压缩的)
struct AVInputFormat; // 输入格式，描述媒体文件的格式和解码方法
struct AVCodecContext; // 编解码器的上下文
struct AVFormatContext; // 媒体文件的上下文

using FrameContainer = std::vector<quint8>; // 存储帧数据容器

// 可以运行在独立线程中的屏幕捕获类
class GDIScreenCapture : public QThread
{
public:
    GDIScreenCapture();
    GDIScreenCapture(const GDIScreenCapture&) = delete;
    GDIScreenCapture& operator=(const GDIScreenCapture&) = delete;
    virtual ~GDIScreenCapture();

public:
    // 获取屏幕的宽度和高度
    virtual quint32 GetWidth() const;   
    virtual quint32 GetHeight() const;

    virtual bool Init(qint64 display_index = 0);
    virtual bool Close();

    // 捕获同一帧屏幕数据，并将RGBA格式的数据存储到容器中，并返回宽度和高度 
    virtual bool CaptureFrame(FrameContainer& rgba,quint32& width,quint32& height);

protected:
    virtual void run() override;

private:
    void StopCapture(); // 停止屏幕捕获
    bool GetOneFrame(); // 捕获一帧数据
    bool Decode(AVFrame* av_frame,AVPacket* av_packet); // 对捕获的帧数据进行解码
private:
    using framPtr = std::shared_ptr<quint8>;
    bool    stop_;
    bool    is_initialzed_;
    quint32 frame_size_;
    framPtr rgba_frame_;
    quint32 width_; // 分辨率
    quint32 height_;
    qint64  video_index_;   // 视频索引
    qint64  framerate_; // 帧率
    std::mutex mutex_;
    // ffmpeg相关的指针，用于音视频格式和编解码操作
    AVInputFormat* input_format_;
    AVCodecContext* codec_context_;
    AVFormatContext* format_context_;

    std::shared_ptr<AVFrame> av_frame_;
    std::shared_ptr<AVPacket> av_packet_;
};
#endif // GDISCREENCAPTURE_H
