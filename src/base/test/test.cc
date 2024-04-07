#include "ThreadPool.h"
#include "CurrentThread.h"
#include "Logging.h"
#include <stdio.h>
#include <unistd.h>
#include <functional>
#include <iostream>

int count = 0;

void showInfo()
{
    std::cout << "test" << CurrentThread::tid() << std::endl;
    LOG_INFO << CurrentThread::tid();
}

void test1()
{
    ThreadPool pool;
    pool.setThreadSize(4);
    for (int i = 0; i < 10; i++)
    {
        pool.add(showInfo);
    }
    pool.add([]{sleep(5);});
    pool.start();
}

void initFunc()
{
    printf("Create thread %d\n", ++count);
}

int main()
{
    test1();

    return 0;
}