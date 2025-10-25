#ifndef WASAPI_CAPTURE_H
#define WASAPI_CAPTURE_H
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <wrl.h>
#include <cstdio>
#include <cstdint>
#include <functional>
#include <mutex>
#include <memory>
#include <thread>

// 音频捕获类
class WASAPICapture
{
public:
    typedef std::function<void(const WAVEFORMATEX *mixFormat, uint8_t *data, uint32_t samples)> PacketCallback; // 定义回调函数
    WASAPICapture();
    WASAPICapture(const WASAPICapture&) = delete;
    WASAPICapture& operator=(const WASAPICapture&) = delete;
    ~WASAPICapture();
    int init();
    int exit();
    int start();
    int stop();
    void setCallback(PacketCallback callback);  // 设置回调函数
    WAVEFORMATEX *getAudioFormat() const
    {
        return m_mixFormat;
    }
private:
    bool m_initialized = false;
    bool m_isEnabeld = false;
    int adjustFormatTo16Bits(WAVEFORMATEX *pwfx);
    int capture();
    const int REFTIMES_PER_SEC = 10000000;
    const int REFTIMES_PER_MILLISEC = 10000;
    
    const IID IID_IAudioClient = __uuidof(IAudioClient);
    const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);
    const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
    const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);

    std::mutex m_mutex; // 线程同步的互斥锁
    uint32_t m_pcmBufSize;  // PCM缓冲区的大小
    uint32_t m_bufferFrameCount;    // 缓冲区中的帧数
    PacketCallback m_callback;  // 回调函数
    WAVEFORMATEX *m_mixFormat = NULL;   // 音频格式信息
    std::shared_ptr<uint8_t> m_pcmBuf; //捕获之后pcm缓存的这个pcmBuf中
    REFERENCE_TIME m_hnsActualDuration; // 计算音频数据的实际持续时间
    std::shared_ptr<std::thread> m_threadPtr;   // 共享指针，捕获音频数据的线程

    // WASAPI的com指针，用于Windows音频系统进行交互
    Microsoft::WRL::ComPtr<IMMDevice> m_device; // 当前选定的音频设备
    Microsoft::WRL::ComPtr<IAudioClient> m_audioClient; // 客户端对象
    Microsoft::WRL::ComPtr<IMMDeviceEnumerator> m_enumerator;   // 枚举系统重所有音频设备的接口
    Microsoft::WRL::ComPtr<IAudioCaptureClient> m_audioCaptureClient; // 捕获音频数据的接口
};
#endif
/*
m_device 选定音频输入设备。
m_enumerator 枚举系统中的音频设备，以便选择一个设备。
m_audioClient 控制音频数据流，管理音频流的开始和结束。
m_audioCaptureClient 从音频流中提取实际的音频数据，供应用程序使用。
*/
