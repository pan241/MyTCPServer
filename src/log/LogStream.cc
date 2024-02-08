#include "LogStream.h"
#include <algorithm>

const char digits[] = "9876543210";

template <typename T>
void LogStream::formatIntger(T num)
{
    if (_buffer.avail() >= kMaxNumericSize)
    {
        char* start = _buffer._cur;
        char* cur = start;
        const char* zero = digits + 9;
        bool negative = (num < 0);

        do {
            int remainder = static_cast<int>(num % 10);
            *(cur++) = zero[remainder];
            num = num / 10;
        } while (num != 0);

        if (negative)
        {
            *(cur++) = '-';
        }
        *cur = '\0';
        std::reverse(start, cur);
        _buffer.add(static_cast<int>(cur - start));
    }
}

LogStream& LogStream::operator<<(short v)
{
    *this << static_cast<int>(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned short v)
{
    *this << static_cast<unsigned int>(v);
    return *this;
}

LogStream& LogStream::operator<<(int v)
{
    formatIntger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned int v)
{
    formatIntger(v);
    return *this;
}

LogStream& LogStream::operator<<(long v)
{
    formatIntger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long v)
{
    formatIntger(v);
    return *this;
}
LogStream& LogStream::operator<<(long long v)
{
    formatIntger(v);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long long v)
{
    formatIntger(v);
    return *this;    
}

LogStream& LogStream::operator<<(float v)
{
    *this << static_cast<double>(v);
    return *this;
}

LogStream& LogStream::operator<<(double v)
{
    if (_buffer.avail() >= kMaxNumericSize)
    {
        int len = snprintf(_buffer.current(), kMaxNumericSize, "%.12g", v);
        _buffer.add(len);
    }
    return *this;
}

LogStream& LogStream::operator<<(char v)
{
    _buffer.append(&v, 1);
    return *this;
}

LogStream& LogStream::operator<<(const void* data)
{
    return operator<<(static_cast<const char*>(data));
}

LogStream& LogStream::operator<<(const char* str)
{
    if (str)
    {
        _buffer.append(str, strlen(str));
    }
    else
    {
        _buffer.append("(null)", 6);
    }
    return *this;
}

LogStream& LogStream::operator<<(const unsigned char* str)
{
    return operator<<(reinterpret_cast<const char*>(str));
}

LogStream& LogStream::operator<<(const std::string str)
{
    _buffer.append(str.c_str(), str.size());
    return *this;
}

LogStream& LogStream::operator<<(const Buffer& buf)
{
    *this << buf.toSting();
    return *this;
}

LogStream& LogStream::operator<<(const charStream& v)
{
    _buffer.append(v._data, v._len);
    return *this;
}
