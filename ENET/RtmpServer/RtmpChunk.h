#include "../EdoyunNet/BufferReader.h"
#include "../EdoyunNet/BufferWriter.h"
#include "RtmpMessage.h"
#include <map>

// 通过封装块(Chunk)的解析和创建逻辑，为RTMP协议的数据传输提供了基础支持。

class RtmpChunk
{
public:
    enum State
    {
        PARSE_HEADER, // 解析头
        PARSE_BODY, // 解析体
    };

    RtmpChunk();
    virtual ~RtmpChunk();

    // 块解析函数，从数据区解析块数据，并将解析出的RTMP信息存储到out_rtmp_msg
    int Parse(BufferReader& in_buffer,RtmpMessage& out_rtmp_msg);
    // 根据RTMPMessage创建块数据的缓冲区
    int CreateChunk(uint32_t csid,RtmpMessage& in_msg, char* buf, uint32_t buf_size);
    // 设置块的大小，分别用于输入和输出方向
    void SetInChunkSize(uint32_t in_chunk_size)
    {
        in_chunk_size_ = in_chunk_size;
    }
    void SetOutChunkSize(uint32_t out_chunk_size)
    {
        out_chunk_size_ = out_chunk_size;
    }
    void Clear()
    {
        rtmp_message_.clear();
    }
    int GetStreamId()const{
        return stream_id_;
    }
protected:
    // 解析块头数据
    int ParseChunkHeader(BufferReader& buffer);
    // 解析块体数据
    int ParseChunkBody(BufferReader& buffer);
    // 创建基本头
    int CreateBasicHeader(uint8_t fmt,uint32_t csid,char* buf);
    // 创建消息头
    int CreateMessageHeader(uint8_t fmt,RtmpMessage& rtmp_msg,char* buf);
private:
    State state_; // 块解析状态(解析头/解析体)
    // 当前块流ID和流ID
    int chunk_stream_id_ = 0;
    static int stream_id_;
    // 输入/输出块大小设置
    uint32_t in_chunk_size_ = 128;
    uint32_t out_chunk_size_ = 128;
    // 用户存储解析出的RTMP信息，用map存储，键为消息ID
    std::map<int, RtmpMessage> rtmp_message_;
    // 块消息头长度数组
    const int KChunkMessageHeaderLenght[4] = {11,7,3,0};
};