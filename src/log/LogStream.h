#ifndef LOGSTREAM_H
#define LOGSTREAM_H

#include "noncopyable.h"
#include <string>
#include <string.h>

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

template<int SIZE>
class FixedBuffer : noncopyable
{
public:
    FixedBuffer() : _cur(_data) {}

    void append(const char* buf, size_t len)
    {
        if (static_cast<size_t>(avail()) > len)
        {
            memcpy(_cur, buf, len);
            _cur += len;
        }
    }

    const char* data() const { return _data; }
    int length() const { return static_cast<int>(end() - _data); }

    char* current() { return _cur; }
    int avail() const { return static_cast<int>(end() - _cur); }
    void add(size_t len) { _cur += len; }

    void reset() { _cur = _data; }
    void bzero() { memset(_data, 0, sizeof(_data)); }

    std::string toSting() const {return std::string(_data, length()); }


private:
    const char* end() const { return _data + sizeof(_data); }

    char _data[SIZE];  
    char* _cur;
};

class charStream : noncopyable
{
public:
    charStream()
        : _data(nullptr), _len(0)
    {}

    explicit charStream(const char* data, int len)
        : _data(data), _len(len)
    {}
    
    const char* _data;
    int _len;
};

class LogStream : noncopyable
{
public:
    using Buffer = FixedBuffer<kSmallBuffer>;

    LogStream& operator<<(bool v)
    {
        _buffer.append(v ? "1" : "0", 1);
        return *this;    
    }

    LogStream& operator<<(short);
    LogStream& operator<<(unsigned short);
    LogStream& operator<<(int);
    LogStream& operator<<(unsigned int);
    LogStream& operator<<(long);
    LogStream& operator<<(unsigned long);
    LogStream& operator<<(long long);
    LogStream& operator<<(unsigned long long);

    LogStream& operator<<(float v);
    LogStream& operator<<(double);

    LogStream& operator<<(char v);
    LogStream& operator<<(const void* data);
    LogStream& operator<<(const char* str);
    LogStream& operator<<(const unsigned char* str);
    LogStream& operator<<(const std::string str);
    LogStream& operator<<(const Buffer& buf);
    LogStream& operator<<(const charStream& v);

    void append(const char* data, size_t len) { _buffer.append(data, len); }
    const Buffer& buffer() const { return _buffer; }
    void resetBuffer() { _buffer.reset(); }

private:
    static const int kMaxNumericSize = 48;

    Buffer _buffer;

    template<typename T>
    void formatIntger(T);

};

#endif