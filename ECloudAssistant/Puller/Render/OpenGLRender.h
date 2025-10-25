#ifndef OPENGLRENDER_H
#define OPENGLRENDER_H
#include <QLabel>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include "AV_Common.h"
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>
#include <QOpenGLPixelTransferOptions>
#include "defin.h"

// 实现了一个OpenGL渲染器，用于渲染视频帧
class OpenGLRender : public QOpenGLWidget, protected  QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    explicit OpenGLRender(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    OpenGLRender(const OpenGLRender&) = delete;
    OpenGLRender& operator=(const OpenGLRender&) = delete;
    virtual ~OpenGLRender();
public:
    virtual void Repaint(AVFramePtr frame);
    void GetPosRation(MouseMove_Body& body);
protected:
    virtual void showEvent(QShowEvent *event);
    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;
private:
    void repaintTexYUV420P(AVFramePtr frame);
    void initTexYUV420P(AVFramePtr frame);
    void freeTexYUV420P();
private:
    QLabel* label_ = nullptr;

    // 存储视频帧的YUV数据
    QOpenGLTexture* texY_ = nullptr;
    QOpenGLTexture* texU_ = nullptr;
    QOpenGLTexture* texV_ = nullptr;
    // 着色器程序
    QOpenGLShaderProgram* program_ = nullptr;
    // 像素传输选项
    QOpenGLPixelTransferOptions options_;
    // 顶点缓冲区对象，顶点数组对象和元素缓冲区对象
    GLuint VBO = 0;
    GLuint VAO = 0;
    GLuint EBO = 0;

    // 存储窗口大小、缩放大小、渲染区域矩形和位置
    QSize   m_size;
    QSizeF  m_zoomSize;
    QRect   m_rect;
    QPointF m_pos;
};
#endif // OPENGLRENDER_H
