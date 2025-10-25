#include <cstdint>
#include <memory>

//msgheader
    struct RtmpMessageHeader
    {
        uint8_t timestamp[3]; // 时间戳
        uint8_t lenght[3];    // 消息长度
        uint8_t type_id;      // 消息类型ID
        uint8_t stream_id[4]; // 消息所属的流ID 小端存储
    };

    struct RtmpMessage
    {
        uint32_t timestamp = 0;
        uint32_t lenght = 0;
        uint8_t type_id = 0;   // 消息类型
        uint32_t stream_id =0;
        uint32_t extend_timestamp = 0;

        uint64_t _timestamp = 0;
        uint8_t codeId = 0;

        uint8_t csid = 0;   // 块流ID
        uint32_t index = 0;   // 当前解析位置
        std::shared_ptr<char> playload = nullptr;

        void Clear()
        {
            index = 0;
            timestamp = 0;
            extend_timestamp = 0;
            if(lenght > 0)
            {
                playload.reset(new char[lenght],std::default_delete<char[]>());
            }
        }

        bool IsCompleted() const 
        {
            if(index == lenght && lenght > 0 && playload != nullptr)
            {
                return true;
            }
            return false;
        }
    };
    