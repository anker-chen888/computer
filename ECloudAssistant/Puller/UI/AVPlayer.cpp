#include "TcpSocket.h"
#include "AVPlayer.h"
#include "EventLoop.h"
#include "SigConnection.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include "defin.h"
#include <QResizeEvent>

//注册信号
Q_DECLARE_METATYPE(AVFramePtr)

AVPlayer::~AVPlayer()
{
    Close();
}

AVPlayer::AVPlayer(EventLoop* loop,QWidget *parent)
    :OpenGLRender(parent)
    ,loop_(loop)
{
    setFocus();
    //设置窗口属性
    this->resize(parentWidget()->size());
    //设置无边框
    this->setWindowFlags(Qt::FramelessWindowHint);
    //背景颜色
    this->setAttribute(Qt::WA_StyledBackground);
    Init();
}

// 初始化
void AVPlayer::Init()
{
    //准备一个音频上下文
    avContext_ = new AVContext();
    //创建这个解封装器
    avDEMuxer_.reset(new AVDEMuxer(avContext_));
    //初始化这个音频播放器
    this->InitAudio(2,44100,16);
    //绑定信号与槽 去播放视频
    connect(this,&AVPlayer::sig_repaint,this,&OpenGLRender::Repaint,Qt::QueuedConnection);
}

void AVPlayer::Close()
{
    stop_ = true;
    if(audioThread_->joinable()) // 检查音频线程是否处于可加入状态
    {
        audioThread_->join();   // 等待音频线程完成其执行
        audioThread_.reset();   // 释放音频线程对象所占用的内存
        audioThread_ = nullptr; // 音频线程指针设置为nullptr
    }
    if(videoThread_->joinable())
    {
        videoThread_->join();
        videoThread_.reset();
        videoThread_ = nullptr;
    }
    if(avDEMuxer_)
    {
        avDEMuxer_.reset();
        avDEMuxer_ = nullptr;
    }
}

bool AVPlayer::Connect(QString ip, uint16_t port, QString code)
{
    TcpSocket tcp_socket;
    tcp_socket.Create();
    if(!tcp_socket.Connect(ip.toStdString(),port))
    {
        qDebug() << "连接信令服务器失败";
        return false;
    }
    qDebug() << "连接信令服务器成功";
    //创建一个信令连接器
    sig_conn_.reset(new SigConnection(loop_->GetTaskSchduler().get(),tcp_socket.GetSocket(),code,SigConnection::CONTROLLING));//控制端
    // 当信令服务器发送停止流指令时的回调函数
    sig_conn_->SetStopStreamCallBack([this](){
        this->HandleStopStream();
    });
    // 当信令服务器发送开始流指令时的回调函数
    sig_conn_->SetStartStreamCallBack([this](const QString& streamAddr){
        return this->HandleStartStream(streamAddr);
    });
    return true;
}

void AVPlayer::audioPlay()
{
    //音频播放，从这个音频帧队列来取pcm去播放
    AVFramePtr frame = nullptr;
    while(!stop_ && avDEMuxer_ && avContext_)
    {
        //判断音频播放大小
        if(!AvailableBytes() || avContext_->audio_queue_.empty())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        //pop
        avContext_->audio_queue_.pop(frame); // 从队列中取出音频帧
        // 播放音频帧
        Write(frame);
    }
}

void AVPlayer::videoPlay()
{
    //视频播放
    AVFramePtr frame = nullptr;
    while(!stop_ && avDEMuxer_ && avContext_)
    {
        if(avContext_->video_queue_.empty())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        //pop
        avContext_->video_queue_.pop(frame);
        sig_repaint(frame);
    }
}

void AVPlayer::resizeEvent(QResizeEvent *event)
{
    OpenGLRender::resizeEvent(event);
}

void AVPlayer::wheelEvent(QWheelEvent *event)
{
    if(!sig_conn_->IsClosed())
    {
        //准备一个事件
        Wheel_Body body;
        //获取这个滚轮值
        body.wheel = event->Wheel;
        QPoint pixeDelta= event->pixelDelta();
        QPoint angleDelta = event->angleDelta();
        body.wheel = (!pixeDelta.isNull() ? (pixeDelta.y() > 0 ? 1 : -1) : (angleDelta.y() > 0 ? 1 : -1));
        //发送这个数据包
        sig_conn_->Send((const char*)&body,body.len);
    }
    QWidget::wheelEvent(event);
}

void AVPlayer::mouseMoveEvent(QMouseEvent *event)
{
    if(!sig_conn_->IsClosed())
    {
        MouseMove_Body body;
        //获取这个x,y比值，后面实现
        this->GetPosRation(body);
        sig_conn_->Send((const char*)&body,body.len);
    }
    QWidget::mouseMoveEvent(event);
}

void AVPlayer::mousePressEvent(QMouseEvent *event)
{
    if(!sig_conn_->IsClosed())
    {
        Mouse_Body body;
        body.type = MouseKeyType::PRESS;
        body.mouseButtons = (MouseType)event->button();
        //发送
        sig_conn_->Send((const char*)&body,body.len);
    }
    QWidget::mousePressEvent(event);
}

void AVPlayer::mouseReleaseEvent(QMouseEvent *event)
{
    if(!sig_conn_->IsClosed())
    {
        Mouse_Body body;
        body.type = MouseKeyType::RELEASE;
        body.mouseButtons = (MouseType)event->button();
        //发送
        sig_conn_->Send((const char*)&body,body.len);
    }
    QWidget::mouseReleaseEvent(event);
}

void AVPlayer::keyPressEvent(QKeyEvent *event)
{
    if(!sig_conn_->IsClosed())
    {
        Key_Body body;
        body.type = MouseKeyType::PRESS;
        body.key = event->key();
        //发送
        sig_conn_->Send((const char*)&body,body.len);
    }
    QWidget::keyPressEvent(event);
}

void AVPlayer::keyReleaseEvent(QKeyEvent *event)
{
    if(!sig_conn_->IsClosed())
    {
        Key_Body body;
        body.key = MouseKeyType::RELEASE;
        body.key = event->key();
        //发送
        sig_conn_->Send((const char*)&body,body.len);
    }
    QWidget::keyReleaseEvent(event);
}

void AVPlayer::HandleStopStream()
{
    //停止拉流
    Close();
}

bool AVPlayer::HandleStartStream(const QString &streamAddr)
{
    //开始拉流
    avDEMuxer_->Open(streamAddr.toStdString());

    //开始启动线程
    audioThread_.reset(new std::thread([this](){
        this->audioPlay();
    }));
    videoThread_.reset(new std::thread([this](){
        this->videoPlay();
    }));
    return true;
}
