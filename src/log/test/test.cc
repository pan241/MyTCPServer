#include "AsyncLogging.h"
#include "Logger.h"
#include "Timestamp.h"

#include <stdio.h>
#include <unistd.h>

static const off_t kRollSize = 1 * 1024 * 1024;
AsyncLogging* g_asyncLog = NULL;

inline AsyncLogging* getAsyncLog()
{
    return g_asyncLog;
}

void test_Logging()
{
    LOG_DEBUG << "debug";
    LOG_INFO << "info";
    LOG_WARN << "warn";
    LOG_ERROR << "error";

    const int n = 10;
    for (int i = 0; i < n; i++)
    {
        LOG_INFO << "Hello, " << i << " abc...xyz";
    }
}

void test_AsyncLogging()
{
    const int n = 1024;
    for (int i = 0; i < n; i++)
    {
        LOG_INFO << "Hello, " << i << " abc...xyz";
    }
}

void asyncLog(const char* msg, int len)
{
    AsyncLogging* logging = getAsyncLog();
    if (logging)
    {
        logging->append(msg, len);
    }
}

int main(int argc, char* argv[])
{
    printf("pid = %d\n", getpid());

    AsyncLogging log(::basename(argv[0]), kRollSize);
    test_Logging();

    sleep(1);

    g_asyncLog = &log;
    Logger::setOutput(asyncLog);
    log.start();

    test_Logging();
    test_AsyncLogging();

    sleep(1);
    log.stop();

    return 0;
}
