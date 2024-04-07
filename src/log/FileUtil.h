#pragma once

#include "noncopyable.h"
#include <string>
#include "Logger.h"

// file < 64KB
class ReadSmallFile : noncopyable
{
public:
    ReadSmallFile(std::string filename);
    ~ReadSmallFile();

    template<typename String>
    int readToString(int maxSize, 
                     String* content, 
                     int64_t* fileSize, 
                     int64_t* modifyTime, 
                     int64_t* createTime);

    int readToBuffer(int * size);
    
    const char* buffer() { return _buf; }

    static const int kBufferSize = 64 * 1024;

private:
    int _fd;
    int _err;
    char _buf[kBufferSize];
};

template<typename String>
int readFile(String filename,
             int maxSize, 
             String* content, 
             int64_t* fileSize = NULL, 
             int64_t* modifyTime = NULL, 
             int64_t* createTime = NULL)
{
    ReadSmallFile file(filename);
    return file.readToString(maxSize, content, fileSize, modifyTime, createTime);
}

class AppendFile : noncopyable
{
public:
    explicit AppendFile(std::string filename);
    ~AppendFile();

    void append(const char* data, size_t len);

    void flush();

    off_t writtenBytes() const { return _writtenBytes; }

private:
    size_t write(const char* data, size_t len);

    FILE* _fp;
    char _buf[64*1024];
    off_t _writtenBytes;
};
