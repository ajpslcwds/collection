/*
 * @Author       : yanli yanli563730@baosight.com
 * @Date         : 2024-09-06 10:03:40
 * @FilePath     : /PF/PFBL/NGVSdemo/src/main.cpp
 * @Copyright (c) 2023 by BAOSIGHT, All Rights Reserved.
 * @Description  :
 */
#include "INGVSAgentManager.h"
#include "socket.h"
#include <cstring>
#include <iostream>
#include <map>
#include <thread>
#include <unistd.h>
#include <vector>

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;
const int VAR5 = 5;
const int VAR15 = 15;

class UserRecvDataCallback : public RecvDataCallback
{

  public:
    void RecvData(string ngvsName, unsigned char *buf, uint32_t length) override
    {
        cout << "recv[" << ngvsName << "]ok" << endl;
        if (ngvsName == "cmd_recv80")
        {
            cout << buf << endl;
        }
    }
};

std::string g_strHeartBeat = "woyouyigexiaomaolv.wotiantianjiushiqi";
void *dsf_ctrl_process_thread_callback(void *arg)
{
    svr_process_t *svr_process = (svr_process_t *)arg;

    while (1)
    {
        auto len = sys_socket_writen_wait((int)svr_process->cli_sock_fd, (void *)g_strHeartBeat.data(),
                                          (int)g_strHeartBeat.size(), 1000);
        cout << "send ok" << len << endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}
void tcpsend()
{
    svr_init(1234, dsf_ctrl_process_thread_callback, WAIT_FOREVER);
}

void tcprecv()
{
    auto fd = socket_client_tcp_create_ipv4("127.0.0.1", 1234, 10, SOCKET_NOBLOCK);
    while (1)
    {
        char buffer[1024];
        int len = sys_socket_readn_wait(fd, buffer, g_strHeartBeat.size(), 1000);
        if (len > 0)
        {
            cout << "recv data:" << buffer << endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int tcpserver()
{
    signal(SIGPIPE, SIG_IGN);
    // 1. 创建 socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket");
        return -1;
    }

    // 允许端口复用
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. 绑定地址和端口
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
    addr.sin_port = htons(1234);

    if (bind(server_fd, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        close(server_fd);
        return -1;
    }

    // 3. 监听
    if (listen(server_fd, 5) < 0)
    {
        perror("listen");
        close(server_fd);
        return -1;
    }

    std::cout << "Server listening on port 1234..." << std::endl;

    while (true)
    { // 4. 接收客户端连接
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (sockaddr *)&client_addr, &client_len);
        if (client_fd < 0)
        {
            perror("accept");
            close(server_fd);
            return -1;
        }

        std::cout << "Client connected: " << inet_ntoa(client_addr.sin_addr) << std::endl;

        // 5. 每秒发送心跳
        std::thread client_th([client_fd]() {
            while (true)
            {
                ssize_t n = send(client_fd, g_strHeartBeat.data(), g_strHeartBeat.size(), 0);
                if (n <= 0)
                {
                    std::cout << "Client disconnected" << std::endl;
                    break;
                }

                std::cout << "Heartbeat sent" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }

            // 6. 关闭 socket
            close(client_fd);
        });
        client_th.detach();
    }
    close(server_fd);
    return 0;
}

int main(int argc, char *argv[])
{
    ServiceProvider serviceProvider;
    INGVSAgentManager *manager = serviceProvider.CreateManager();
    map<string, string> send;
    map<string, string> recv;

    if (argc < 2)
    {
        cout << "please input recv or send" << endl;
        return 0;
    }
    std::string path = std::string("./") + argv[1];
    manager->Initial(path.c_str(), send, recv);
    UserRecvDataCallback rdp1;
    if (argv[1][0] == 'r')
    {
        manager->RegisterAll(&rdp1);
    }
    manager->StartAll();

    if (argv[1][0] == 's')
    {
        // std::thread t([]() { tcpserver(); });
        std::thread t([]() { tcpsend(); });
        t.detach();
    }

    std::string buffer(1024, '1');
    while (true)
    {
        if (argv[1][0] == 's')
        {
            for (auto it : send)
            {
                auto ret = manager->Publish(it.second, (unsigned char *)buffer.data(), 1024);
                if (ret)
                    cout << "publish success" << endl;
                else
                    cout << "publish fail" << endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}