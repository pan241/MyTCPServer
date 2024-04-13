#ifndef LOGGING_H
#define LOGGING_H

#include <string.h>
#include <functional>
#include "Timestamp.h"
#include "LogStream.h"

class Logger
{
public:
    enum LogLevel
    {
    	TRACE,
    	DEBUG,
    	INFO,
    	WARN,
    	ERROR,
    	FATAL,
    	LEVEL_COUNT,
    };
    
    class SourceFile
    {
    public:
    	template<int N>
    	SourceFile(const char (&arr)[N])
			: _data(arr), _size(N - 1)
		{
			const char* slash = strchr(_data, '/');
			if (slash)
			{
				_data = slash + 1;
				_size -= static_cast<int>(_data - arr);
			}
		}

		explicit SourceFile(const char* filename)
			: _data(filename)
		{
			const char* slash = strchr(filename, '/');
			if (slash)
			{
				_data = slash + 1;
			}
			_size = static_cast<int>(strlen(_data));
		}
    	
    	const char* _data;
		int _size;
    };

	Logger(SourceFile file, int line);
	Logger(SourceFile file, int line, LogLevel level);
	Logger(SourceFile file, int line, LogLevel level, const char* func);
	Logger(SourceFile file, int line, bool toAbort);
	~Logger();

	LogStream& stream() { return _impl._stream; }

	static LogLevel logLevel();
	static void setLogLevel(LogLevel level);

	using OutputFunc = std::function<void(const char* msg, int len)>;
	using FlushFunc =  std::function<void()>;
	static void setOutput(OutputFunc);
	static void setFlush(FlushFunc);

private:
	class Impl
	{
	public:
		using LogLevel = Logger::LogLevel;
		Impl(LogLevel level, int savedErrno, const SourceFile file, int line);
		void formatTime();
		void finish();

		Timestamp _time;
		LogStream _stream;
		LogLevel _level;
		int _line;
		SourceFile _basename;
	};

	Impl _impl;
};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::logLevel()
{
	return g_logLevel;
}

const char* getErrnoMsg(int savedErrno);

#define LOG_DEBUG if (Logger::logLevel() <= Logger::DEBUG) \
	Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()
#define LOG_INFO if (Logger::logLevel() <= Logger::INFO) \
	Logger(__FILE__, __LINE__).stream()
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::ERROR).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::FATAL).stream()

#endif
