#include <atomic>
#include <cstring> // For memset
#include <iostream>
#include <mutex>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

#define SHM_SIZE sizeof(std::atomic<int>) + sizeof(std::mutex) // 共享内存大小

// 定义一个结构体来包含原子计数器和互斥锁
struct SharedData
{
    std::atomic<int> counter;
    int i_counter;
};

void increment(SharedData *data)
{
    for (int i = 0; i < 1000; ++i)
    {
        // 使用 mutex 锁保护对原子变量的访问
        data->counter.fetch_add(1);
        ++data->i_counter;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main()
{
    // 创建共享内存段
    key_t key = ftok("shmfile", 65); // 生成唯一键
    int shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);

    if (shmid < 0)
    {
        perror("shmget failed");
        return 1;
    }

    // 将共享内存附加到当前进程的地址空间
    SharedData *sharedData = static_cast<SharedData *>(shmat(shmid, nullptr, 0));

    if (sharedData == (SharedData *)(-1))
    {
        perror("shmat failed");
        return 1;
    }

    // 初始化共享数据
    new (&sharedData->counter) std::atomic<int>(0);
    // new (&sharedData->counter) int(0);

    // 创建线程以模拟多个进程
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i)
    {
        threads.emplace_back(std::thread(increment, sharedData));
    }

    for (auto &t : threads)
    {
        t.join();
    }

    std::cout << "Final counter value: " << sharedData->counter << std::endl;
    std::cout << "Final i_counter value: " << sharedData->i_counter << std::endl;

    // 分离共享内存
    shmdt(sharedData);

    // 删除共享内存
    shmctl(shmid, IPC_RMID, nullptr);

    return 0;
}
