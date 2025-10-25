#include "RtmpChunk.h"
#include <string.h>

int RtmpChunk::stream_id_ = 0;

RtmpChunk::RtmpChunk()
{
    state_ = PARSE_HEADER;
    chunk_stream_id_ = -1;
    ++stream_id_ ;
}

RtmpChunk::~RtmpChunk()
{
}

// 从输入缓冲区中解析RTMP块数据，并将解析出的RTMP消息存储到out_rtmp_msg
int RtmpChunk::Parse(BufferReader &in_buffer, RtmpMessage &out_rtmp_msg)
{
    int ret = 0;
    if(!in_buffer.ReadableBytes())
    {
        return 0;
    }

    if(state_ == PARSE_HEADER)
    {
        ret = ParseChunkHeader(in_buffer);
    }
    else
    {
        ret = ParseChunkBody(in_buffer);
        if(ret > 0 && chunk_stream_id_ >= 0)  //解析成功
        {
            auto& rtmp_msg = rtmp_message_[chunk_stream_id_]; // 根据块流ID找到对应RTMP消息
            if(rtmp_msg.index == rtmp_msg.lenght) //说明message是一个完整的
            {
                if(rtmp_msg.timestamp >= 0xffffff)
                {
                    rtmp_msg._timestamp += rtmp_msg.extend_timestamp;
                }
                else
                {
                    rtmp_msg._timestamp += rtmp_msg.timestamp;
                }

                out_rtmp_msg = rtmp_msg; // 赋值过去
                chunk_stream_id_ = -1;
                rtmp_msg.Clear();
            }
        }
    }
    return ret;
}

// 根据输入的RTMP消息创建块数据
// csid：块流ID（Chunk Stream ID）。
// in_msg：输入的RTMP消息，包含消息的负载数据、时间戳等信息。
// buf：用于存储创建的块数据的缓冲区。
// buf_size：缓冲区的大小。

int RtmpChunk::CreateChunk(uint32_t csid, RtmpMessage &in_msg, char *buf, uint32_t buf_size)
{
    // 记录当前缓冲区偏移量、当前消息负载数据的偏移量
    uint32_t buf_offset = 0, playload_offset = 0; 
    uint32_t capacity = in_msg.lenght + in_msg.lenght / out_chunk_size_ * 5;
    if(buf_size < capacity)
    {
        return -1;
    }

    buf_offset += CreateBasicHeader(0,csid,buf + buf_offset);  // 创建基本头
    buf_offset += CreateMessageHeader(0,in_msg,buf + buf_offset); // 创建信息头
    // 处理扩展时间戳
    if(in_msg._timestamp >= 0xffffff)
    {
        WriteUint32BE((char*)buf + buf_offset,(uint32_t)in_msg.extend_timestamp);
        buf_offset += 4;
    }
    // 分块写入消息负载
    while (in_msg.lenght > 0)
    {
        if(in_msg.lenght > out_chunk_size_)
        {
            // 将消息数据复制过去
            memcpy(buf + buf_offset,in_msg.playload.get() + playload_offset,out_chunk_size_);
            playload_offset += out_chunk_size_;
            buf_offset += out_chunk_size_;
            in_msg.lenght -= out_chunk_size_;

            buf_offset += CreateBasicHeader(3,csid,buf + buf_offset); // 这里表示与上一个块的消息一致，所以不创建块消息头
            if(in_msg._timestamp >= 0xffffff)
            {
                WriteUint32BE(buf + buf_offset,(uint32_t)in_msg.extend_timestamp);
                buf_offset += 4;
            }
        }
        else //最后一个包
        {
            memcpy(buf + buf_offset,in_msg.playload.get() + playload_offset,in_msg.lenght);
            buf_offset += in_msg.lenght;
            in_msg.lenght = 0;
            break;
        }
    }
    
    return buf_offset;
}

// 块头解析 包括基本头和信息头
/*
1.解析基本头，获取块格式类型（fmt）和块流ID（csid）。
2.根据fmt确定消息头的长度，并解析消息头。
3.更新RTMP消息的相关信息，包括消息长度、类型ID、流ID和时间戳。
4.切换解析状态为解析块体，并更新缓冲区的读取位置。
*/
int RtmpChunk::ParseChunkHeader(BufferReader &buffer)
{
    uint32_t bytes_used = 0;    // 使用过的字节数
    uint8_t* buf = (uint8_t*)buffer.Peek(); // 读取数据
    uint32_t buf_size = buffer.ReadableBytes(); // 数据大小

    uint8_t flags = buf[bytes_used];
    uint8_t fmt = (flags >> 6);
    if(fmt >= 4) //fmt 0-3
    {
        return -1;
    }
    bytes_used += 1;
    uint8_t csid = flags & 0x3f;
    if(csid == 0) //2字节
    {
        if(buf_size < (bytes_used + 2))
        {
            return 0;
        }

        csid += buf[bytes_used] + 64;
        bytes_used += 1;
    }
    else if(csid  == 1) //3字节
    {
        if(buf_size < (bytes_used + 3))
        {
            return 0;
        }
        csid += buf[bytes_used + 1] * 255 + buf[bytes_used] + 64;
        bytes_used += 2;
    }

    uint32_t header_len = KChunkMessageHeaderLenght[fmt];
    if(buf_size < (header_len + bytes_used))
    {
        return 0;
    }

    RtmpMessageHeader header;
    memset(&header,0,sizeof(RtmpMessageHeader));
    memcpy(&header,buf + bytes_used,header_len);
    bytes_used += header_len;

    auto& rtmp_msg = rtmp_message_[csid]; // 取出RTMP数据
    chunk_stream_id_ = rtmp_msg.csid = csid; // 设置块流ID

    if(fmt == 0 || fmt == 1)
    {
        uint32_t lenght = ReadUint24BE((char*)header.lenght);
        if(rtmp_msg.lenght != lenght || !rtmp_msg.playload)
        {
            rtmp_msg.lenght = lenght;
            rtmp_msg.playload.reset(new char[rtmp_msg.lenght],std::default_delete<char[]>());
        }
        rtmp_msg.index = 0;
        rtmp_msg.type_id = header.type_id;
    }
    // 处理流ID
    if(fmt == 0)
    {
        rtmp_msg.stream_id = ReadUint32LE((char*)header.stream_id);
    }

    uint32_t timestamp = ReadUint24BE((char*)header.timestamp);
    uint32_t extend_timestamp = 0;
    if(timestamp >= 0xffffff || rtmp_msg.timestamp >= 0xffffff)
    {
        if(buf_size < (bytes_used + 4))
        {
            return 0;
        }
        extend_timestamp = ReadUint32BE((char*)buf + bytes_used);
        bytes_used += 4;
    }

    if(rtmp_msg.index == 0)
    {
        if(fmt == 0)
        {
            rtmp_msg._timestamp = 0;
            rtmp_msg.timestamp = timestamp;
            rtmp_msg.extend_timestamp = extend_timestamp;
        }
        else
        {
            if(rtmp_msg.timestamp >= 0xffffff)
            {
                rtmp_msg.extend_timestamp += extend_timestamp;
            }
            else
            {
                rtmp_msg.timestamp += timestamp;
            }
        }
    }

    state_ = PARSE_BODY;
    buffer.Retrieve(bytes_used);
    return bytes_used;
}

// 块体解析
/*
初始化变量并检查块流ID。
获取RTMP消息和计算块大小。
检查缓冲区大小是否足够。
复制块体数据到RTMP消息的负载缓冲区。
更新消息索引和解析状态。
更新缓冲区的读取位置。
*/
int RtmpChunk::ParseChunkBody(BufferReader &buffer)
{
    uint32_t bytes_used = 0;
    uint8_t* buf = (uint8_t*)buffer.Peek();
    uint32_t buf_size = buffer.ReadableBytes();

    if(chunk_stream_id_ < 0) //小于0，说明chunk头解析失败
    {
        return -1;
    }

    auto& rtmp_msg = rtmp_message_[chunk_stream_id_];
    uint32_t chunk_size = rtmp_msg.lenght - rtmp_msg.index;
    if(chunk_size > in_chunk_size_)
    {
        chunk_size = in_chunk_size_;
    }

    if(buf_size < (chunk_size + bytes_used))
    {
        return 0;
    }

    if(rtmp_msg.index + chunk_size > rtmp_msg.lenght)
    {
        return -1;
    }

    memcpy(rtmp_msg.playload.get() + rtmp_msg.index,buf + bytes_used,chunk_size);
    bytes_used += chunk_size;
    rtmp_msg.index += chunk_size;

    if(rtmp_msg.index >= rtmp_msg.lenght ||
    rtmp_msg.index % in_chunk_size_ == 0)
    { //解析出一个完整的message
        state_ = PARSE_HEADER;
    }
    buffer.Retrieve(bytes_used);
    return bytes_used;
}

// 创建基本头
// 块格式、块流ID
int RtmpChunk::CreateBasicHeader(uint8_t fmt, uint32_t csid, char *buf)
{  
    int len= 0;

    if(csid >= 64 + 255) //说明这个basic头占3字节
    {
        buf[len++] = (fmt << 6) | 1;
        buf[len++] = (csid - 64) & 0xff;
        buf[len++] = ((csid - 64) >> 8) & 0xff;
    }
    else if(csid >= 64)//占2字节
    {
        buf[len++] = (fmt << 6) | 0;
        buf[len++] = (csid - 64) & 0xff;
    }
    else //1字节
    {
        buf[len++] = (fmt << 6) | csid;
    }
    return len;
}

// 创建信息头
int RtmpChunk::CreateMessageHeader(uint8_t fmt, RtmpMessage &rtmp_msg, char *buf)
{
    int len = 0;
    if(fmt <= 2)
    {
        if(rtmp_msg._timestamp < 0xffffff)
        {
            WriteUint24BE((char*)buf,(uint32_t)rtmp_msg._timestamp);
        }
        else{
            WriteUint24BE((char*)buf,0xffffff);
        }
        len += 3;
    }
    if(fmt <= 1)
    {
        WriteUint24BE((char*)buf + len,rtmp_msg.lenght);
        len += 3;
        buf[len++] = rtmp_msg.type_id; // Message stream id
    }
    if(fmt == 0)
    {
        WriteUint32LE((char*)buf + len,rtmp_msg.stream_id);
        len += 4;
    }
    return len;
}
