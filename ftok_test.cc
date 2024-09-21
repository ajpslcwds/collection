/**
 * Filename        ftok_test.cc
 * Copyright       Shanghai Baosight Software Co., Ltd.
 * Description     
 *
 * Author          wuzheqiang
 * Version         09/20/2024	wuzheqiang	Initial Version
 */


#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief          
 * @param [in]       
 * @param [out]      
 * @return         
 * @version        2024/09/20	wuzheqiang	Initial Version
 */
int main() {
    key_t key;
    int shmid;

    // 生成IPC键
    key = ftok("tmp", 'A');  // 使用/tmp目录和'A'字符生成一个key
    if (key == -1) {
        perror("ftok failed");
        exit(EXIT_FAILURE);
    }

    printf("key: 0x%x\n",key);

    // 创建共享内存
    shmid = shmget(key, 1024, 0666 | IPC_CREAT);  // 创建或获取一个1KB大小的共享内存
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    printf("Shared memory created with id: %d\n", shmid);
    return 0;
}
