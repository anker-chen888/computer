#ifndef AUIDO_BUFFER_H
#define AUIDO_BUFFER_H
#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <mutex>

// 音频缓冲区，用于存储和处理音频数据
class AudioBuffer
{
public:
    AudioBuffer(uint32_t size = 1024)
        : _bufferSize(size)
    {
        _buffer.resize(size);
    }

    ~AudioBuffer()
    {

    }
    // 向缓冲区写入数据，会通过互斥锁来确保线程安全
    int write(const char *data, uint32_t size)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        uint32_t bytes = writableBytes();

        if (bytes < size)
        {
            size = bytes;
        }

        if (size > 0)
        {
            memcpy(beginWrite(), data, size); // 使用memcpy将数据从data复制到缓冲区的写入位置
            _writerIndex += size;
        }

        retrieve(0);
        return size;
    }
    // 从缓冲区读取数据
    int read(char *data, uint32_t size)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (size > readableBytes())
        {
            retrieve(0);
            return -1;
        }

        memcpy(data, peek(), size);
        retrieve(size);
        return size;
    }
    // 获取缓冲区大小
    uint32_t size()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return readableBytes();
    }
    // 清除缓冲区
    void clear()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        retrieveAll();
    }

private:
    // 可读数据大小
    uint32_t readableBytes() const
    {
        return _writerIndex - _readerIndex;
    }
    // 可写数据大小
    uint32_t writableBytes() const
    {
        return _buffer.size() - _writerIndex;
    }
    // 返回当前读取位置的指针
    char* peek()
    {
        return begin() + _readerIndex;
    }

    const char* peek() const
    {
        return begin() + _readerIndex;
    }

    void retrieveAll()
    {
        _writerIndex = 0;
        _readerIndex = 0;
    }
    // 从缓冲区移除已读取的数据
    void retrieve(size_t len)
    {
        if (len > 0)
        {
            if (len <= readableBytes())
            {
                _readerIndex += len;
                // 所有缓冲区数据已被读取
                if (_readerIndex == _writerIndex)
                {
                    _readerIndex = 0;
                    _writerIndex = 0;
                }
            }
        }
        // 都大于0 进行一个可读和可写数据的优化
        if (_readerIndex > 0 && _writerIndex > 0)
        {
            _buffer.erase(_buffer.begin(), _buffer.begin() + _readerIndex);
            _buffer.resize(_bufferSize);
            _writerIndex -= _readerIndex;
            _readerIndex = 0;
        }
    }

    void retrieveUntil(const char* end)
    {
        retrieve(end - peek());
    }

    char* begin()
    {
        return &*_buffer.begin();
    }

    const char* begin() const
    {
        return &*_buffer.begin();
    }

    char* beginWrite()
    {
        return begin() + _writerIndex;
    }

    const char* beginWrite() const
    {
        return begin() + _writerIndex;
    }

    std::mutex _mutex;
    uint32_t _bufferSize = 0;
    std::vector<char> _buffer;
    size_t _readerIndex = 0;
    size_t _writerIndex = 0;
};
#endif
