#ifndef LOGGING_H
#define LOGGING_H


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
    	
    private:
    
    	
    };

}



#endif
