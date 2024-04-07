#include "Logger.h"
#include "CurrentThread.h"

namespace ThreadInfo
{
    __thread char t_errnobuf[512];
    __thread char t_time[64];
    __thread time_t t_lastSecond;
};

class T
{
public:
    T(const char* str, unsigned len)
        : _str(str), _len(len)
    {}

    const char* _str;
    const unsigned _len;

};

inline LogStream& operator<<(LogStream& s,T v)
{
    s.append(v._str, v._len);
    return s;
}

inline LogStream& operator<<(LogStream& s, const Logger::SourceFile& v)
{
    s.append(v._data, v._size);
    return s;
}

const char* getErrnoMsg(int savedErrno)
{
    return strerror_r(savedErrno, ThreadInfo::t_errnobuf, sizeof(ThreadInfo::t_errnobuf));
}

const char* LogLevelName[Logger::LogLevel::LEVEL_COUNT]
{
    "TRACE ",
    "DEBUG ",
    "INFO ",
    "WARN ",
    "ERROR ",
    "FATAL ",
};

Logger::LogLevel initLogLogLevel()
{
    return Logger::INFO;
}

Logger::LogLevel g_logLevel = initLogLogLevel();

static void defaultOutput(const char* msg, int len)
{
    fwrite(msg, len, sizeof(char), stdout);
}

static void defaultFlush()
{
    fflush(stdout);
}

Logger::Impl::Impl(LogLevel level, int savedErrno, const SourceFile file, int line)
    :_time(Timestamp::now()),
     _stream(),
	 _level(level),
	 _line(line),
	 _basename(file)
{
    formatTime();
    CurrentThread::tid();
    _stream << T(LogLevelName[level], 6);
    if (savedErrno != 0)
    {
        _stream << getErrnoMsg(savedErrno) << " (errno =" << savedErrno << ") ";
    }
}

void Logger::Impl::formatTime()
{
    int64_t microSecondSinceEpoch = _time.microSecondSinceEpoch();
    time_t seconds = static_cast<time_t>(microSecondSinceEpoch / Timestamp::kMicroSecondsPerSecond);
    int microseconds = static_cast<int>(microSecondSinceEpoch % Timestamp::kMicroSecondsPerSecond);

    struct tm* tm_time = localtime(&seconds);

    snprintf(ThreadInfo::t_time, sizeof(ThreadInfo::t_time), "%4d/%02d/%02d %02d:%02d:%02d",
        tm_time->tm_year + 1900,
        tm_time->tm_mon + 1,
        tm_time->tm_mday,
        tm_time->tm_hour,
        tm_time->tm_min,
        tm_time->tm_sec);   

    ThreadInfo::t_lastSecond = seconds;
    char buf[32] = {0};
    snprintf(buf, sizeof(buf), "%06d", microseconds);
    _stream << T(ThreadInfo::t_time, 17) << T(buf, 7);

} 

void Logger::Impl::finish()
{
    _stream << " - " << _basename << ':' << _line << '\n';
}

Logger::OutputFunc  g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;

Logger::Logger(SourceFile file, int line) 
    : _impl(INFO, 0, file, line)
{}

Logger::Logger(SourceFile file, int line, LogLevel level) 
    : _impl(level, 0, file, line)
{}

Logger::Logger(SourceFile file, int line, LogLevel level, const char* func) 
: _impl(level, 0, file, line)
{
    _impl._stream << func << ' ';
}

Logger::Logger(SourceFile file, int line, bool toAbort) 
    : _impl(toAbort ? FATAL : ERROR, errno, file, line)
{}

Logger::~Logger()
{
    _impl.finish();
    const LogStream::Buffer& buf(stream().buffer());
    g_output(buf.data(), buf.length());
    if (_impl._level == FATAL)
    {
        g_flush();
        abort();
    }
}

void setLogLevel(Logger::LogLevel level)
{
    g_logLevel = level;
}

void Logger::setOutput(OutputFunc out)
{
    g_output = out;
}

void Logger::setFlush(FlushFunc flush)
{
    g_flush = flush;
} 