#ifndef AVPLAYER_H
#define AVPLAYER_H
#include "OpenGLRender.h"
#include "AudioRender.h"
#include "AVDEMuxer.h"

class EventLoop;
class SigConnection;
// 音视频播放器的核心类，负责从RTMP流中提取音视频数据并进行解码、渲染
class AVPlayer : public OpenGLRender ,public AudioRender
{
    Q_OBJECT
public:
    ~AVPlayer();
    explicit AVPlayer(EventLoop* loop,QWidget* parent = nullptr); // 构造函数
    bool Connect(QString ip,uint16_t port,QString code);    // 连接到RTMP流服务器，参数包括服务器IP、端口和认证码
signals:
    void sig_repaint(AVFramePtr frame);    // 处理流的开始，接受流地址并初始化音视频解码和渲染
protected:
    void audioPlay();   // 音频播放 多线程
    void videoPlay();   // 视频播放 多线程
    void Init();
    void Close();
    // 重写了Qt的事件处理方法，用于响应窗口大小变化、鼠标时间、键盘事件
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
private:
    void HandleStopStream();
    bool HandleStartStream(const QString& streamAddr);
private:
    bool stop_ = false; // 用于控制播放器的停止状态
    EventLoop* loop_;   // 事件循环对象
    AVContext* avContext_ = nullptr;    // 音视频上下文对象
    std::shared_ptr<SigConnection> sig_conn_;   // 信号连接对象
    std::unique_ptr<AVDEMuxer> avDEMuxer_ = nullptr;    // 解复用器对象，用于从RTMP流中提取音视频数据
    std::unique_ptr<std::thread> audioThread_ = nullptr;    // 音频线程
    std::unique_ptr<std::thread> videoThread_ = nullptr;    // 视频线程
};
#endif // AVPLAYER_H
