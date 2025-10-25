#include <cstdint>
#include <array>
#include <string>

//需要1字节对齐
#pragma pack(push,1)


// 主要用来定义客户端与服务器之间通信的指令、应答码、状态码以及数据包的结构
//cmd命令
enum Cmd : uint16_t
{
    JOIN = 5, // 客户端加入房间
    OBTAINSTREAM, // 客户端获取流
    CREATESTREAM, // 客户端创建流
    PLAYSTREAM,   // 播放流
    DELETESTREAM, // 删除流
    // 下面是鼠标和键盘事件
    MOUSE,
    MOUSEMOVE,
    KEY,
    WHEEL,
};

//应答 服务器对客户端请求的相应结果
enum ResultCode
{
    SUCCESSFUL, // 请求成功
    ERROR,      // 请求失败
    REQUEST_TIMEOUT ,       // 请求超时
    ALREADY_REDISTERED ,    // 验证失败
    USER_DISAPPEAR,    
    ALREADY_LOGIN,
    VERFICATE_FAILED,
};

//客户端状态
enum RoleState
{
    IDLE,       //说明客户端已经创建房间，但没有去拉流也没有推流
    NONE,       //说明当前客户端没有创建房间
    CLOSE,      //客户端断开连接
    PULLER,     //客户端进入拉流模式
    PUSHER,     //客户端开始推流
};

struct packet_head
{
    packet_head():len(-1),cmd(-1){}
    uint16_t len;
    uint16_t cmd;
};

//包体
//创建房间
struct Join_body : public packet_head
{
    Join_body() : packet_head()
    {
        cmd = JOIN;
        len = sizeof(Join_body);
        id.fill('\0');
    }
    // 设置Id
    void SetId(const std::string& str)
    {
        str.copy(id.data(),id.size(),0);
    }
    // 获取Id
    std::string GetId()
    {
        return std::string(id.data());
    }
    std::array<char,10> id; 
};

//创建房间应答
struct JoinReply_body : public packet_head
{
    JoinReply_body() : packet_head()
    {
        cmd = JOIN;
        len = sizeof(JoinReply_body);
        result = ERROR;
    }
    //设置结果
    void SetCode(const ResultCode code)
    {
        result = code;
    }
    ResultCode result;
};

//获取流
struct ObtainStream_body : public packet_head
{
    ObtainStream_body() : packet_head()
    {
        cmd = OBTAINSTREAM;
        len = sizeof(ObtainStream_body);
        id.fill('\0');
    }
    void SetId(const std::string& str)
    {
        str.copy(id.data(),id.size(),0);
    }
    std::string GetId()
    {
        return std::string(id.data());
    }
    std::array<char,10> id; // 控制对方的Id
};

//获取流应答
struct ObtainStreamReply_body : public packet_head
{
    ObtainStreamReply_body() : packet_head()
    {
        cmd = OBTAINSTREAM; // 获取流
        len = sizeof(ObtainStreamReply_body);
        result = ERROR;
    }
    void SetCode(const ResultCode code)
    {
        result = code;
    }
    ResultCode result;
};

//创建流    客户端向服务器发送的创建流的请求包
struct CreateStream_body : public packet_head
{
    CreateStream_body() : packet_head()
    {
        cmd = CREATESTREAM;
        len = sizeof(CreateStream_body);
    }
};

//创建流应答 流地址和结果  服务器对客户端创建流请求的回复包
struct CreateStreamReply_body : public packet_head
{
    CreateStreamReply_body() : packet_head()
    {
        cmd = CREATESTREAM; // 设置指令类型为创建流
        len = sizeof(CreateStreamReply_body); // 包长
        result = ERROR; // 默认错误
        streamAddres.fill('\0');  // 初始流地址
    }
    // 设置流地址
    void SetstreamAddres(const std::string& str)
    {
        str.copy(streamAddres.data(),streamAddres.size(),0);
    }
    std::string GetstreamAddres()
    {
        return std::string(streamAddres.data());
    }
    void SetCode(const ResultCode code)
    {
        result = code;
    }
    ResultCode result;
    std::array<char,70> streamAddres;  
};

//播放流 提供播放流地址
// 客户端向服务器发送播放流的请求，需要携带流地址信息
struct PlayStream_body : public packet_head
{
    PlayStream_body() : packet_head()
    {
        cmd = PLAYSTREAM;
        len = sizeof(PlayStream_body);
        result = ERROR;
        streamAddres.fill('\0');
    }
    // 设置流地址
    void SetstreamAddres(const std::string& str)
    {
        str.copy(streamAddres.data(),streamAddres.size(),0);
    }
    std::string GetstreamAddres()
    {
        return std::string(streamAddres.data());
    }
    void SetCode(const ResultCode code)
    {
        result = code;
    }
    ResultCode result;
    std::array<char,70> streamAddres;  // 存储流地址
};

// 服务器对客户端播放流请求的回复
struct PlayStreamReplay_body : public packet_head
{
    PlayStreamReplay_body() : packet_head()
    {
        cmd = PLAYSTREAM;
        len = sizeof(PlayStreamReplay_body);
        result = ERROR;
    }
    void SetCode(const ResultCode code)
    {
        result = code;
    }
    ResultCode result;
};

struct DeleteStream_body : public packet_head
{
    DeleteStream_body():packet_head()
    {
        cmd = DELETESTREAM;
        streamCount = -1;
        len = sizeof(DeleteStream_body);
    }
    void SetStreamCount(const int count)
    {
        streamCount = count;
    }
    int streamCount;//推流的时候，如果发现拉流数量为0,我们就需要停止推流，如果流数量不为0，就说明还有客户端连接，不能停止推流
};

//鼠标键盘信息
enum MouseType : uint8_t
{
    NoButton = 0,
    LeftButton = 1,
    RightButton = 2,
    MiddleButton = 4,
    XButton1 = 8,
    XButton2 = 16,
};

//鼠标/键盘按下还是松开
enum MouseKeyType : uint8_t
{
    PRESS,  // 按下
    RELESE, // 松开
};

//键盘消息
struct Key_body : public packet_head
{
    Key_body():packet_head(){
        cmd = KEY;
        len = sizeof(Key_body);
    }
    //键值和类型
    uint16_t key;
    MouseKeyType type;  // 按下or松开
};

//滚轮消息
struct Wheel_body : public packet_head
{
    Wheel_body():packet_head(){
        cmd = WHEEL;
        len = sizeof(Wheel_body);
    }
    //值
    uint8_t wheel;
};

//鼠标移动
struct MouseMove_body : public packet_head
{
    MouseMove_body():packet_head(){
        cmd = MOUSEMOVE;
        len = sizeof(MouseMove_body);
    }
    //需要x,y比值
    uint8_t xl_ratio;
    uint8_t xr_ratio;
    uint8_t yl_ratio;
    uint8_t yr_ratio;
};

// 鼠标信息结构体
struct Mouse_body : public packet_head
{
    Mouse_body():packet_head()
    {
        cmd = MOUSE;
        len = sizeof(Mouse_body);
    }
    MouseKeyType type;
    MouseType mouseType;
};

#pragma pack(pop)