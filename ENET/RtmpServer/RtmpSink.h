#ifndef _RTMPSINK_H_
#define _RTMPSINK_H_
#include <cstdint>
#include <memory>
#include "amf.h"

// RtmpSink 类提供了一个接口规范，用于处理 RTMP 流媒体数据的接收和发送。
class RtmpSink
{
public:
    RtmpSink(){}
    virtual ~RtmpSink(){}
    // 发送元数据的虚函数
    virtual bool SendMetaData(AmfObjects metaData) {return true;}
    // 发送媒体数据的纯虚函数
    virtual bool SendMediaData(uint8_t type,uint64_t timestamp,std::shared_ptr<char> playload,uint32_t playload_size) = 0; //纯虚函数
    // 查询是否为播放器虚函数
    virtual bool IsPlayer() {return false;}
    // 查询是否为推流器的虚函数
    virtual bool IsPublisher() {return false;}
    // 查询是否正在推流的虚函数
    virtual bool IsPlaying() {return false;}
    // 获取ID的纯虚函数
    virtual bool IsPublishing() {return false;}

    virtual uint32_t GetId() = 0;

};
#endif