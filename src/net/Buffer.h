#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <assert.h>
#include <string.h>
#include <unistd.h>

/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
class Buffer
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        : _buffer(kCheapPrepend + initialSize),
          _readIndex(kCheapPrepend),
          _writeIndex(kCheapPrepend)
    {}

    size_t readableBytes() const { return _writeIndex - _readIndex; }
    size_t writableBytes() const { return _buffer.size() - _writeIndex;}
    size_t prependableByes() const { return _readIndex; }

    void swap(Buffer& rhs)
    {
        _buffer.swap(rhs._buffer);
        std::swap(_readIndex, rhs._readIndex);
        std::swap(_writeIndex, rhs._writeIndex);
    }

    const char* peek() const { return begin() + _readIndex; }

    std::string retrieveAllAsString() { return retrieveAsString(readableBytes()); }

    std::string retrieveAsString(size_t len)
    {
        assert(len <= readableBytes());
        std::string result(peek(), len);
        retrieve(len); // 缓冲区复位
        return result;
    }

    void retrieve(size_t len)
    {
        // assert(len <= readableBytes());
        if (len <= readableBytes())
        {
            _readIndex += len;
        }
        else
        {
            rertrieveAll();
        }
    }

    void rertrieveAll()
    {
        _readIndex = _writeIndex = kCheapPrepend;
    }

     void retrieveUntil(const char* end)
    {
        assert(peek() <= end);
        assert(end <= beginWrite());
        retrieve(end - peek());
    }

    const char* findCRLF() const
    {
        const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
        return crlf == beginWrite() ? nullptr : crlf;
    }

    const char* findCRLF(const char* start) const
    {
        const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
        return crlf == beginWrite() ? nullptr : crlf;
    }

    const char* findEOL() const
    {
        const void* eol = memchr(peek(), '\n', readableBytes());
        return static_cast<const char*>(eol);
    }

    void append(const char* data, size_t len)
    {
        ensureWritableBytes(len);
        std::copy(data, data+len, beginWrite());
        hasWritten(len);
    }

    void append(const std::string& str) { append(str.data(), str.size()); }
 
    void ensureWritableBytes(size_t len)
    {
        if (writableBytes() < len)
        {
            makeSpace(len);
        }
        assert(writableBytes() >= len);
    }

    char* beginWrite() { return begin() + _writeIndex; }

    const char * beginWrite() const { return begin() + _writeIndex; }

    void hasWritten(size_t len) { _writeIndex += len; }

    void unwrite(size_t len) { _writeIndex -= len;}

    ssize_t readFd(int fd, int* saveErrno);
    ssize_t writeFd(int fd, int* saveErrno);

private:
    char* begin() { return &*_buffer.begin(); }
    const char* begin() const { return &*_buffer.begin(); }

    void makeSpace(size_t len)
    {
        if (writableBytes() + prependableByes() < len + kCheapPrepend)
        {
            _buffer.resize(_writeIndex + len);
        }
        else
        {
            size_t readable = readableBytes();
            std::copy(begin() + _readIndex,
                    begin() + _writeIndex,
                    begin() + kCheapPrepend);
            _readIndex = kCheapPrepend;
            _writeIndex = _readIndex + readable;
        }
    }

    std::vector<char> _buffer;
    size_t _readIndex;
    size_t _writeIndex;

    static const char kCRLF[];
};
