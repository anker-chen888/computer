#ifndef VIDEO_CONVERTER_H
#define VIDEO_CONVERTER_H
#include "AV_Common.h"
struct SwsContext;

// 用于将视频帧从一个像素格式和分辨率转换为另一种像素格式和分辨率
class VideoConverter
{
public:
    VideoConverter();
    virtual ~VideoConverter();
    VideoConverter(const VideoConverter&) = delete;
    VideoConverter& operator=(const VideoConverter&) = delete;
public:
    bool Open(qint32 in_width,qint32 in_height,AVPixelFormat in_format,
              qint32 out_width,qint32 out_height,AVPixelFormat out_format);
    void Close();

    qint32 Convert(AVFramePtr in_frame,AVFramePtr& out_frame);
private:
    qint32 width_;  // 存储输出的宽度和高度
    qint32 height_;
    AVPixelFormat format_; // 存储输出的像素格式
    SwsContext* swsContext_; // 执行实际的像素格式转换
};
#endif // VIDEO_CONVERTER_H
