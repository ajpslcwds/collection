#include <cstdlib>
#include <iostream>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define SHM_SIZE sizeof(int) // 共享内存大小
#define SEM_KEY 1234         // 信号量键
#define SHM_KEY 5678         // 共享内存键

// 操作信号量的函数
void sem_operation(int semid, int op)
{
    struct sembuf sop;
    sop.sem_num = 0; // 操作第一个信号量
    sop.sem_op = op; // 操作类型：1表示增加，-1表示减少
    sop.sem_flg = 0; // 无特殊操作
    if (semop(semid, &sop, 1) == -1)
    {
        perror("semop failed");
        exit(EXIT_FAILURE);
    }
}

int main()
{
    // 创建共享内存
    int shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid < 0)
    {
        perror("shmget failed");
        return 1;
    }

    // 将共享内存附加到当前进程的地址空间
    int *shared_value = (int *)shmat(shmid, nullptr, 0);
    if (shared_value == (int *)(-1))
    {
        perror("shmat failed");
        return 1;
    }

    // 初始化共享内存中的值
    *shared_value = 0;

    // 创建信号量
    int semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (semid < 0)
    {
        perror("semget failed");
        return 1;
    }

    // 初始化信号量为 1（表示可用）
    if (semctl(semid, 0, SETVAL, 1) == -1)
    {
        perror("semctl failed");
        return 1;
    }

    // 创建子进程
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork failed");
        return 1;
    }

    const int iterations = 10; // 每个进程的迭代次数

    for (int i = 0; i < iterations; ++i)
    {
        // 请求进入临界区
        sem_operation(semid, -1); // P 操作，减少信号量

        // 临界区开始
        std::cout << "Process " << (pid == 0 ? "Child" : "Parent") << " incrementing value: " << *shared_value
                  << " -> ";
        (*shared_value)++;
        std::cout << *shared_value << std::endl;
        // 临界区结束

        // 释放临界区
        sem_operation(semid, 1); // V 操作，增加信号量

        sleep(1); // 模拟一些工作
    }

    // 清理资源
    shmdt(shared_value); // 分离共享内存
    if (pid > 0)
    {
        // 只有父进程会执行这段代码
        wait(nullptr);                    // 等待子进程结束
        shmctl(shmid, IPC_RMID, nullptr); // 删除共享内存
        semctl(semid, 0, IPC_RMID);       // 删除信号量
    }

    return 0;
}
