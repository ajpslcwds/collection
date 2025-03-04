
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statfs.h>
#include <time.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace std;

#define AINFO std::cout
#define ADEBUG std::cout
#define STACK_BUF_LEN 1024

int func(int start)
{
    for (int i = start * 10; i < start * 10 + 10; i++)
    {
        cout << this_thread::get_id() << ":" << start << ":" << i << endl;
        this_thread::sleep_for(std::chrono::seconds(1));
    }
    return start;
}

void test_thread()
{
    int i = 0;
    std::thread t[5];
    for (i = 0; i < 5; i++)
    {
        t[i] = std::thread(&func, i);
    }

    for (i = 0; i < 5; i++)
    {
        t[i].join();
    }
    cout << "test_thread" << this_thread::get_id() << endl;
}

void test_async()
{
    std::future<int> fut[4];
    std::future<int> fut1;
    for (int i = 0; i < 4; i++)
    {
        // std::async(std::launch::async, std::bind(&func, i));  // 每次一个线程，逐个调用
        // auto fut = std::async(std::launch::async, std::bind(&func, i));   // 和上面一样，每次一个线程，逐个调用

        // fut1 = std::async(std::launch::async, std::bind(&func, i)); // 两个线程跑两个，完了在跑两个

        fut[i] = std::async(std::launch::async, std::bind(&func, i)); // 符合期望的四个线程同时跑
    }

    for (int i = 0; i < 4; i++)
    {
        fut[i].detach();
    }

    for (int i = 0; i < 4; i++)
    {
        cout << i << ":" << fut[i].get() << endl;
    }

    while (1)
    {
        cout << "test_async" << this_thread::get_id() << endl;
        this_thread::sleep_for(std::chrono::seconds(2));
    }
}

int main()
{
    // test_thread();
    test_async();
    return 0;
}