#ifndef _AMF_H_
#define _AMF_H_
#include <string>
#include <memory>
#include <map>
#include <unordered_map>

typedef enum
{ 
	AMF0_NUMBER = 0, // 数字(double)
	AMF0_BOOLEAN, // 布尔
	AMF0_STRING, 	// 字符串
	AMF0_OBJECT, 	// 对象
	AMF0_MOVIECLIP,		/* reserved, not used */
	AMF0_NULL, 		// null
	AMF0_UNDEFINED, // 未定义
	AMF0_REFERENCE, 	// 引用
	AMF0_ECMA_ARRAY, 	// 数组
	AMF0_OBJECT_END,	// 对象结束
	AMF0_STRICT_ARRAY, 	// 严格的数组
	AMF0_DATE, 			// 日期
	AMF0_LONG_STRING, 	// 长字符串
	AMF0_UNSUPPORTED,	// 未支持
	AMF0_RECORDSET,		/* reserved, not used */
	AMF0_XML_DOC, 		// xml文档
	AMF0_TYPED_OBJECT,  // 有类型的对象
	AMF0_AVMPLUS,		/* switch to AMF3 */
	AMF0_INVALID = 0xff	// 无效的
} AMF0DataType;


typedef enum
{
	AMF_NUMBER, // 数字
	AMF_BOOLEAN,  // 布尔值
	AMF_STRING, // 字符串
} AmfObjectType;

// 定义了AMF对象的结果，包含数据类型和对应的数据值
struct AmfObject
{  
	AmfObjectType type;

	std::string amf_string;
	double amf_number;
	bool amf_boolean;    

	AmfObject()
	{
        
	}
	// 初始化字符串类型AMF对象
	AmfObject(std::string str)
	{
		this->type = AMF_STRING; 
		this->amf_string = str; 
	}
	// 初始化数字类型的AMF对象
	AmfObject(double number)
	{
		this->type = AMF_NUMBER; 
		this->amf_number = number; 
	}
};

// 键是字符串、值是AmfObject
typedef std::unordered_map<std::string, AmfObject> AmfObjects;

// 实现AMF数据的解码功能
class AmfDecoder
{
public:    
	/* n: 解码次数 */
    int decode(const char *data, int size, int n=-1); // 解码数据

	// 重置解码器状态
    void reset()
    {
        m_obj.amf_string = "";
        m_obj.amf_number = 0;
        m_objs.clear();
    }

	// 获取解码后的字符串值
    std::string getString() const
    { return m_obj.amf_string; }

	// 获取解码后的数字值
    double getNumber() const
    { return m_obj.amf_number; }

	// 检查是否包含指定键的AMF对象
    bool hasObject(std::string key) const
    { return (m_objs.find(key) != m_objs.end()); }

	// 获取指定键的AMF对象
    AmfObject getObject(std::string key) 
    { return m_objs[key]; }

    AmfObject getObject() 
    { return m_obj; }

	//获取所有AMF对象
    AmfObjects getObjects() 
    { return m_objs; }
    
private:    
    static int decodeBoolean(const char *data, int size, bool& amf_boolean);	//解码布尔值。
    static int decodeNumber(const char *data, int size, double& amf_number);	// 解码数字
    static int decodeString(const char *data, int size, std::string& amf_string);	// 解码字符串
    static int decodeObject(const char *data, int size, AmfObjects& amf_objs);	// 解码AMF对象
    static uint16_t decodeInt16(const char *data, int size);	// 解码16位整数
    static uint32_t decodeInt24(const char *data, int size);	// 解码24位整数
    static uint32_t decodeInt32(const char *data, int size);	// 解码32位整数

    AmfObject m_obj;	  // 当前解码的AMF对象
    AmfObjects m_objs;    // 存储所有解码的AMF对象
};

// 实现AMF数据的编码功能
class AmfEncoder
{
public:
	AmfEncoder(uint32_t size = 1024); // 初始化编码器
	virtual ~AmfEncoder();
     
	void reset()
	{
		m_index = 0;
	}
    // 获取编码后的数据
	std::shared_ptr<char> data()
	{
		return m_data;
	}
	// 获取编码数据的大小
	uint32_t size() const 
	{
		return m_index;
	}
    // 编码字符串、数字、布尔值、AMF对象、ECMA格式的AMF对象
	void encodeString(const char* str, int len, bool isObject=true);
	void encodeNumber(double value);
	void encodeBoolean(int value);
	void encodeObjects(AmfObjects& objs);
	void encodeECMA(AmfObjects& objs);
     
private:
	// 编码8、16、24、32位整数
	void encodeInt8(int8_t value);
	void encodeInt16(int16_t value);
	void encodeInt24(int32_t value);
	void encodeInt32(int32_t value); 

	// 重新分配缓冲区大小
	void realloc(uint32_t size);

	std::shared_ptr<char> m_data;	// 存储编码数据的缓冲区
	uint32_t m_size  = 0;	// 缓冲区大小
	uint32_t m_index = 0;	// 当前写入位置
};
#endif