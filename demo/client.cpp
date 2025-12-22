#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

int connect_to_server(const char *ip, int port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        return -1;
    }
    // int flags = fcntl(sockfd, F_GETFL, 0);
    // fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);

    if (connect(sockfd, (sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int main()
{
    const char *server_ip = "127.0.0.1";
    const int server_port = 1234;
    std::string g_strHeartBeat = "woyouyigexiaomaolv.wotiantianjiushiqi";

    while (true)
    {
        std::cout << "Trying to connect..." << std::endl;

        int sockfd = connect_to_server(server_ip, server_port);
        if (sockfd < 0)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        std::cout << "Connected to server" << std::endl;

        char buffer[1024];

        while (true)
        {
            memset(buffer, 0, sizeof(buffer));
            ssize_t n = recv(sockfd, buffer, g_strHeartBeat.size(), 0);
            if (n > 0)
            {
                std::cout << "Recv: " << buffer << std::endl;
            }
            else
            {
                std::cout << "Server disconnected, reconnecting..." << std::endl;
                close(sockfd);
                break;
            }
        }

        // 断线后等一会再重连
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
