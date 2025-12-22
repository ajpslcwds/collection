/**
 * @file socket.h
 * @brief
 * @author wwk (1162431386@qq.com)
 * @version 1.0
 * @date 2024-09-13
 *
 * @copyright Copyright (c) 2024  by  宝信
 *
 * @par 修改日志:
 * <table>
 * <tr><th>Date       <th>Version <th>Author  <th>Description
 * <tr><td>2024-09-13     <td>1.0     <td>wwk   <td>修改?
 * </table>
 */
#ifndef __SOCKET_H_
#define __SOCKET_H_
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define SOCKET_BLOCK 0
#define SOCKET_NOBLOCK 1

#define SOCK_LISTEN_NUM (10)

#define NO_WAIT 0
#define WAIT_FOREVER (-1)

#define true 1
#define false 0

#ifndef SAFE_CLOSE
#define SAFE_CLOSE(fd)  \
    do {                \
        if (-1 != fd) { \
            close(fd);  \
            fd = -1;    \
        }               \
    } while (0)
#endif

#ifndef SAFE_FREE
#define SAFE_FREE(ptr)     \
    do {                   \
        if (NULL == ptr) { \
            free(ptr);     \
            ptr = NULL;    \
        }                  \
    } while (0)
#endif

struct svr_process_t {
    int cli_sock_fd;
    struct sockaddr_in cliaddr;
};

#ifdef DEBUG
#define SOCKET_PRINTF(fmt, args...)                                         \
    do {                                                                    \
        printf("[%s %s] FILE:[%s]--[%s]LINE:[%d]:" fmt, __DATE__, __TIME__, \
               __FILE__, __FUNCTION__, __LINE__, ##args);                   \
    } while (0);
#else
#define SOCKET_PRINTF(...) printf(__VA_ARGS__)
#endif

typedef void* (*svr_process_thread)(void*);

/*
 * @Function: svr_init
 * @Description:服务端初始化函数
 * @Input:arg  port 端口  func 数据处理自定义回调函数，可在该函数实现收发
 * uWaitMsec  等待链接超时  isBlock 是否阻塞 WAIT_FOREVER 阻塞 NO_WAIT 非阻塞
 * @Output:NULL
 * @Return:成功 0  失败 -1
 */
int svr_init(unsigned short int port, svr_process_thread func, int uWaitMsec);

/*
 * @Function: socket_connect_wait
 * @Description:close socket
 * @Input:sHostName  ip  port 端口  uWaitMsec  超时时间  isBlock 是否阻塞
 * SOCKET_BLOCK 阻塞 SOCKET_NOBLOCK 非阻塞
 * @Output:NULL
 * @Return: NULL
 */
int socket_client_tcp_create_ipv4(const char* sHostName, int uPort,
                                  int uWaitMsec, int isBlock);

/*
 * @Function: sys_socket_readn_wait
 * @Description:从对端接受固定字节的数据
 * @Input: sockfd：描述符  pbuf：接受缓冲区  buflen：接受字节长
 * uWaitMsec：超等等待时间
 * @Output:NULL
 * @Return:实际接收到数据的长度
 */
int sys_socket_readn_wait(int sockfd, void* pbuf, int buflen,
                          unsigned int uWaitMsec);

/*
 * @Function: sys_socket_read_wait
 * @Description:尝试从对端接受数据
 * @Input: sockfd：描述符  pbuf：接受缓冲区  buflen：缓冲区大小
 * uWaitMsec：超等等待时间
 * @Output:NULL
 * @Return:实际接收到数据的长度
 */
int sys_socket_read_wait(int sockfd, void* pbuf, int bufsize,
                         unsigned int uWaitMsec);

/**@brief	向对端发送数据
 * @param[in]  sockfd 调用sys_socket_create()返回的套接字
 * @param[in]  pbuf 接收缓冲区
 * @param[in]  buflen 发送数据的长度
 * @param[in]  uWaitMsec:等待时间
 * @param[out] 无
 * @return	  实际发送到数据的长度
 */
int sys_socket_writen_wait(int sockfd, void* pbuf, int buflen, int uWaitMsec);

/*
 * @Function: sys_socket_writen
 * @Description:向对端发送数据
 * @Input: sockfd：描述符  pbuf：接受缓冲区  buflen：发送的字节数
 * @Output:NULL
 * @Return:实际发送的数据的长度
 */
int sys_socket_writen(int sockfd, void* pbuf, int buflen);

int setPthreadAttr(pthread_attr_t* attr, int priority, size_t stacksize,
                   int bRealTime);

#if defined(__cplusplus)
}
#endif

#endif