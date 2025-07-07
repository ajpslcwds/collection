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

constexpr size_t TIME_SECOND_LENGTH = 4;
constexpr size_t TIME_MILLISECOND_LENGTH = 2;
constexpr size_t TIMESTAMP_LENGTH = TIME_SECOND_LENGTH + TIME_MILLISECOND_LENGTH;
constexpr size_t QUALITY_LENGTH = 2;
constexpr uint16_t DEFAULT_RECORD_QUALITY = 192;

redisContext *context_ = nullptr;
#define FREE_REDIS_REPLY(reply)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        if (nullptr != (reply))                                                                                        \
        {                                                                                                              \
            freeReplyObject(reply);                                                                                    \
        }                                                                                                              \
    } while (false)

int32_t get_redis_data(const std::string &key, std::string &value)
{

    redisReply *reply = (redisReply *)redisCommand(context_, "GET %s", key.c_str());
    if (reply == nullptr || context_->err)
    {
        std::cout << "redisCommand failed:" << context_->err << std::endl;
        FREE_REDIS_REPLY(reply);
        return -1;
    }

    if (reply->type == REDIS_REPLY_STRING && reply->str != nullptr)
    {
        value = std::string(reply->str, reply->len);
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

int32_t set_redis_data(const std::string &key, std::string &value)
{

    redisReply *reply = (redisReply *)redisCommand(context_, "SET %s %b", key.c_str(), value.c_str(), value.length());
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

template <typename T> int test_write(const std::string &redis_key, const T value)
{
    uint32_t varLength = sizeof(T);
    auto cur_milliseconds =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();
    uint32_t cur_seconds = cur_milliseconds / 1000;
    cur_milliseconds = cur_milliseconds % 1000;

    uint32_t vtq_length = varLength + TIMESTAMP_LENGTH + QUALITY_LENGTH;
    std::shared_ptr<char> pVtqData = std::shared_ptr<char>(new char[vtq_length], std::default_delete<char[]>());
    memcpy(pVtqData.get(), &value, varLength);
    size_t writePos = varLength;
    memcpy(pVtqData.get() + writePos, &cur_seconds, TIME_SECOND_LENGTH);
    writePos += TIME_SECOND_LENGTH;
    memcpy(pVtqData.get() + writePos, &cur_milliseconds, TIME_MILLISECOND_LENGTH);
    writePos += TIME_MILLISECOND_LENGTH;
    memcpy(pVtqData.get() + writePos, &DEFAULT_RECORD_QUALITY, QUALITY_LENGTH);

    std::string redis_value = std::string(pVtqData.get(), vtq_length);
    if (0 != set_redis_data(redis_key, redis_value))
    {
        std::cout << "get redis data failed" << std::endl;
        return -1;
    }
    return 0;
}
template <typename T> int test_read(const std::string &redis_key)
{
    std::string redis_value = "";
    if (0 != get_redis_data(redis_key, redis_value))
    {
        std::cout << "get redis data failed,key:" << redis_key << std::endl;
        return -1;
    }

    T value;
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

    return 0;
}

template <> int test_read<std::string>(const std::string &redis_key)
{
    std::string redis_value = "";
    if (0 != get_redis_data(redis_key, redis_value))
    {
        std::cout << "get redis data failed" << std::endl;
        return -1;
    }

    std::string value;
    int32_t time_s = 0;
    uint16_t time_ms = 0;
    int16_t quality = 0;

    int value_total_length = redis_value[0];
    int value_length = redis_value[1];
    std::cout << "value_total_length:" << value_total_length << std::endl;
    std::cout << "value_length:" << value_length << std::endl;
    value.assign(redis_value.c_str() + 2, value_length);
    memcpy(&time_s, redis_value.c_str() + value_total_length + 2, sizeof(time_s));
    memcpy(&time_ms, redis_value.c_str() + value_total_length + 2 + sizeof(time_s), sizeof(time_ms));
    memcpy(&quality, redis_value.c_str() + value_total_length + 2 + sizeof(time_s) + sizeof(time_ms), sizeof(quality));

    std::cout << "value:" << value.c_str() << std::endl;
    std::cout << "time_s:" << time_s << std::endl;
    std::cout << "time_ms:" << time_ms << std::endl;
    std::cout << "quality:" << quality << std::endl;

    return 0;
}

int main(int argc, char *argv[])
{
    context_ = redisConnect("127.0.0.1", 6380);
    if (argc < 2)
    {
        std::cout << "please input tag_name" << std::endl;
        return -1;
    }

    std::string tag_name = argv[1];
    // test_write<int32_t>(std::string("STD::TWZQ"), 0xffff);
    test_read<std::string>(tag_name);

    redisFree(context_);
    return 0;
}