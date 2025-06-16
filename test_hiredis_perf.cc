/**
 * Filename        test_hiredis.cc
 * Copyright       Shanghai Baosight Software Co., Ltd.
 * Description
 *
 * Author          wuzheqiang
 * Version         05/12/2025    wuzheqiang    Initial Version
 * /usr/bin/g++ -std=c++17 -fdiagnostics-color=always -g /home/wzq/code/collection/test_hiredis.cc -o
 * /home/wzq/code/collection/test_hiredis -std=c++17 -lpthread -lhiredis
 **************************************************************/
#include <chrono>
#include <cstring>
#include <hiredis/hiredis.h>
#include <iostream>
#include <memory>
#include <stdint.h>
#include <string>
#include <thread>
#include <csignal>

constexpr size_t TIME_SECOND_LENGTH = 4;
constexpr size_t TIME_MILLISECOND_LENGTH = 2;
constexpr size_t TIMESTAMP_LENGTH = TIME_SECOND_LENGTH + TIME_MILLISECOND_LENGTH;
constexpr size_t QUALITY_LENGTH = 2;
constexpr uint16_t DEFAULT_RECORD_QUALITY = 192;

struct TestData
{
    int id[20];
    float f_data[20];
    char c_data[64];
};

redisContext *context_ = nullptr;
#define FREE_REDIS_REPLY(reply)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        if (nullptr != (reply))                                                                                        \
        {                                                                                                              \
            freeReplyObject(reply);                                                                                    \
        }                                                                                                              \
    } while (false)

int32_t get_redis_data(const char *key, char *value)
{
    try
    {
        redisReply *reply = (redisReply *)redisCommand(context_, "GET %s", key);
        if (reply == nullptr || context_->err)
        {
            std::cout << "redisCommand failed:" << context_->err << std::endl;
            FREE_REDIS_REPLY(reply);
            return -1;
        }

        if (reply->type == REDIS_REPLY_STRING && reply->str != nullptr)
        {
            value = reply->str;
            std::cout << "read_success~" << std::endl;
        }
        else
        {
            std::cout << "Unexpected reply type: " << reply->type << std::endl;
            FREE_REDIS_REPLY(reply);
            return -1;
        }

        FREE_REDIS_REPLY(reply);
        return 0;
    }
    catch (std::exception &e)
    {
        std::cout << "Exception caught: " << e.what() << std::endl;
        return -1;
    }
    catch (...)
    {
        std::cout << "Unknown exception caught" << std::endl;
        return -1;
    }
    return 0;
}

int32_t set_redis_data(const char *key, const char *buffer, const size_t len)
{
    try
    {
        redisReply *reply = (redisReply *)redisCommand(context_, "SET %s %b", key, buffer, len);
        if (reply == nullptr)
        {
            if (context_ && context_->err)
            {
                std::cout << "Redis command error: " << context_->errstr;
            }
            FREE_REDIS_REPLY(reply);
            return -1;
        }

        if (reply->type == REDIS_REPLY_ERROR)
        {
            std::cout << "Redis reply error: " << reply->str;
            FREE_REDIS_REPLY(reply);
            return -1;
        }
        FREE_REDIS_REPLY(reply);

        return 0;
    }
    catch (std::exception &e)
    {
        std::cout << "Exception caught: " << e.what() << std::endl;
        return -1;
    }
    catch (...)
    {
        std::cout << "Unknown exception caught" << std::endl;
        return -1;
    }
    return 0;
}

constexpr int NUM = 10000;
void test_read(bool flag)
{
    char buffer[1024];

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM; i++)
    {
        if (flag)
            redisReconnect(context_);
        get_redis_data("wzq_key", buffer);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "test_read executed in " << duration.count() << " ms, NUM =" << NUM << ", is reconnect:" << flag
              << std::endl;
}

void test_write(bool flag)
{
    TestData td;
    char buffer[1024];
    memcpy(buffer, &td, sizeof(td));

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM; i++)
    {
        if (flag)
            redisReconnect(context_);
        set_redis_data("wzq_key", buffer, sizeof(td));
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "test_write executed in " << duration.count() << " ms, NUM =" << NUM << ", is reconnect:" << flag
              << std::endl;
}
int main(int args, char *argv[])
{
    std::signal(SIGPIPE, SIG_IGN);
    if (args == 2)
        context_ = redisConnect("127.0.0.1", 6380);
    else
        context_ = redisConnectUnix("/etc/redis/redis.sock");
    // test_read(true);
    // test_write(true);

    test_read(false);
    // test_write(false);
    redisFree(context_);
    return 0;
}