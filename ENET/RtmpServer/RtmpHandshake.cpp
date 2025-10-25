#include "RtmpHandshake.h"
#include <random>
#include <string.h>

RtmpHandshake::RtmpHandshake(State state)
{
    handshake_state_ = state;
}

RtmpHandshake::~RtmpHandshake()
{
}

// C2和S2：4字节时间戳、4字节从对端读到的时间戳、1528字节随机数
int RtmpHandshake::Parse(BufferReader &in_buffer, char *res_buf, uint32_t res_buf_size)
{
    uint8_t* buf = (uint8_t*)in_buffer.Peek(); // 读取接收到的数据
    uint32_t buf_size = in_buffer.ReadableBytes(); // 读取数据的大小
    uint32_t pos = 0;   // 记录当前解析的位置
    uint32_t res_size = 0; // 记录构建的响应数据的大小
    std::random_device rd;

    if(handshake_state_ == HANDSHAKE_S0S1S2) //由客户端处理 拉流
    {
        if(buf_size < (1 + 1536 + 1536))
        {
            return res_size;
        }

        if(buf[0] != 3)
        {
            return -1;
        }

        pos += 1 + 1536 + 1536;
        res_size = 1536; //需要发送的数据大小
        //准备C2
        memcpy(res_buf, buf + 1, 1536); //我们将这个C2传回去
        handshake_state_ = HANDSHAKE_COMPLETE;
    }
    else if(handshake_state_ == HANDSHAKE_C0C1) //由服务端处理 推流
    {
        if(buf_size < 1 + 1536) //C0C1
        {
            return res_size;
        }
        else
        {
            if(buf[0] != 3)
            {
                return -1;
            }

            pos += 1537;
            res_size = 1 + 1536 + 1536;
            memset(res_buf,0,res_size); //返回S0S1S2
            res_buf[0] = 3;

            char*p = res_buf; p += 9;
            for(int i = 0;i < 1528; i++)
            {
                *p++ = rd();
            }
            memcpy(p,buf + 1,1536);
            handshake_state_ = HANDSHAKE_C2;
        }
    }
    else if(handshake_state_ == HANDSHAKE_C2)//服务器处理C2
    {
        if(buf_size < 1536) //C2不完整
        {
            return res_size;
        }
        else
        {
            pos += 1536;
            handshake_state_ = HANDSHAKE_COMPLETE;
        }
    }
    in_buffer.Retrieve(pos); // 重置
    return res_size; // 返回响应数据大小
}

// C0 1字节 C1 4字节时间戳 4字节的0 1528字节的随机数=1536
int RtmpHandshake::BuildC0C1(char *buf, uint32_t buf_size) //客户端需要创建C0C1
{
    uint32_t size = 1 + 1536; //C0C1
    memset(buf,0,size);
    buf[0] = 3;//版本为3

    std::random_device rd; // 生成随机数
    uint8_t* p = (uint8_t*)buf; p += 9; // 前面9个不填充
    for(int i = 0;i < 1528;i++)
    {
        *p++ = rd();
    }
    return size;
}
