#include "lockfree_queue.hpp"
#include <iostream>
#include <thread>
#include <vector>

int test_thread()
{
    LockFreeQueue<int> queue;

    // 生产者线程
    auto producer = [&queue](int t) {
        for (int i = 0; i < 500; ++i)
        {
            queue.push(t * 1000 + i);
        }
    };

    // 消费者线程
    auto consumer = [&queue](int t) {
        int value;
        while (queue.pop(value))
        {
            // std::cout << "Thread " << t << " popped value: " << value << std::endl;
            printf("Thread %d popped value: %d\n", t, value);
        }
    };

    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;

    // 启动多个生产者
    for (int i = 0; i < 10; ++i)
    {
        producers.emplace_back(producer, i);
    }

    // 启动多个消费者
    for (int i = 0; i < 4; ++i)
    {
        consumers.emplace_back(consumer, i);
    }

    // 等待线程完成
    for (auto &p : producers)
    {
        p.join();
    }
    for (auto &c : consumers)
    {
        c.join();
    }

    return 0;
}

int test()
{
    LockFreeQueue<int> queue;
    for (int i = 0; i < 10000; ++i)
    {
        queue.push(i);
    }

    int value;
    while (queue.pop(value))
    {
        printf("popped value: %d\n", value);
    }
    return 0;
}

int main(){
    test_thread();
    test();
    return 0;
}
