/**
 * Filename        test_hiredis.cc
 * Copyright       Shanghai Baosight Software Co., Ltd.
 * Description     
 *
 * Author          wuzheqiang
 * Version         05/12/2025    wuzheqiang    Initial Version
 * /usr/bin/g++ -std=c++17 -fdiagnostics-color=always -g /home/wzq/code/collection/test_hiredis.cc -o /home/wzq/code/collection/test_hiredis -std=c++17 -lpthread -lhiredis
 **************************************************************/
#include <cstring>
#include <hiredis/hiredis.h>
#include <iostream>
#include <stdint.h>
#include <string>

redisContext *context_ = nullptr;
int32_t get_redis_data(const std::string &key, std::string &value)
{

    redisReply *reply = (redisReply *)redisCommand(context_, "GET %s", key.c_str());
    if (reply == nullptr || context_->err)
    {
        std::cout << "redisCommand failed:" << context_->err << std::endl;
        return -1;
    }

    value = std::string(reply->str, reply->len);
    return 0;
}
int main()
{
    context_ = redisConnect("127.0.0.1", 6380);
    {
        std::string redis_key = "STD::TINT";
        std::string redis_value = "";
        if (0 != get_redis_data(redis_key, redis_value))
        {
            std::cout << "get redis data failed" << std::endl;
            return -1;
        }

        uint16_t value = {0};
        int32_t time_s = 0;
        uint16_t time_ms = 0;
        int16_t quality = 0;
        memcpy(&value, redis_value.c_str(), sizeof(value));
        memcpy(&time_s, redis_value.c_str() + sizeof(value), sizeof(time_s));
        memcpy(&time_ms, redis_value.c_str() + sizeof(value) + sizeof(time_s), sizeof(time_ms));
        memcpy(&quality, redis_value.c_str() + sizeof(value) + sizeof(time_s) + sizeof(time_ms), sizeof(quality));

        std::cout << "value:" << value << std::endl;
        std::cout << "time_s:" << time_s << std::endl;
        std::cout << "time_ms:" << time_ms << std::endl;
        std::cout << "quality:" << quality << std::endl;
    }
    {
        std::string redis_key = "STD::STR9";
        std::string redis_value = "";
        if (0 != get_redis_data(redis_key, redis_value))
        {
            std::cout << "get redis data failed" << std::endl;
            return -1;
        }

        char value[432] = {0};
        int32_t time_s = 0;
        uint16_t time_ms = 0;
        int16_t quality = 0;
        memcpy(value, redis_value.c_str(), sizeof(value));
        memcpy(&time_s, redis_value.c_str() + sizeof(value), sizeof(time_s));
        memcpy(&time_ms, redis_value.c_str() + sizeof(value) + sizeof(time_s), sizeof(time_ms));
        memcpy(&quality, redis_value.c_str() + sizeof(value) + sizeof(time_s) + sizeof(time_ms), sizeof(quality));

        std::cout << "value:" << value << std::endl;
        std::cout << "time_s:" << time_s << std::endl;
        std::cout << "time_ms:" << time_ms << std::endl;
        std::cout << "quality:" << quality << std::endl;
    }

    redisFree(context_);
    return 0;
}