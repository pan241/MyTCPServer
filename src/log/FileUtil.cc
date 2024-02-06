#include "FileUtil.h"
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>

AppendFile::AppendFile(std::string filename)
    : _fp(::fopen(filename.c_str(), "ae")),
      _writtenBytes(0)
{
    assert(_fp);
}

AppendFile::~AppendFile()
{
    ::fclose(_fp);
}

void AppendFile::append(const char* data, size_t len)
{
    size_t written = 0;

    while (written != len)
    {
        size_t remain = len - written;
        size_t n = write(data + written, remain);
        if (n != remain)
        {
            int err = ferror(_fp);
            if (err)
            {
                fprintf(stderr, "AppendFile::append() failed %s\n", getErrnoMsg(err));
            }
        }
        written += n;
    }

    _writtenBytes += written;
}

void AppendFile::flush()
{
    ::fflush(_fp);
}

size_t AppendFile::write(const char* data, size_t len)
{
    return ::fwrite_unlocked(data, 1, len, _fp);
}

ReadSmallFile::ReadSmallFile(std::string filename)
    : _fd(::open(filename.c_str(), O_RDONLY | O_CLOEXEC)),
      _err(0)
{
    _buf[0] = '\0';
    if (_fd < 0)
    {
        _err = errno;
    }
}

ReadSmallFile::~ReadSmallFile()
{
    if (_fd >= 0)
    {
        ::close(_fd);
    }
}