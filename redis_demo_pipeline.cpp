#include <chrono>
#include <hiredis/hiredis.h>
#include <iostream>
#include <sstream>
#include <string.h>

const std::string base_key = "this_is_a_test_key_";
const std::string bin_base_key = "this_is_a_test_bin_key_";

const int ARRER_NUM = 20;
int PIPELINE_SIZE = 100; // 每个 Pipeline 中的命令数量

struct TestData
{
    int id[ARRER_NUM];
    float f_data[ARRER_NUM];
    char c_data[32]; // save key
};

class TestRedis
{
  public:
    TestRedis(const char *unix_socket_path)
    {
        context = redisConnectUnix(unix_socket_path);

        if (context == nullptr || context->err)
        {
            if (context)
            {
                std::cerr << "Error: " << context->errstr << "\n";
                redisFree(context);
            }
            else
            {
                std::cerr << "Can't allocate redis context\n";
            }
            throw std::runtime_error("Failed to connect to Redis");
        }
    }

    TestRedis(const std::string &host, int port)
    {
        context = redisConnect(host.c_str(), port);
        if (context == nullptr || context->err)
        {
            if (context)
            {
                std::cerr << "Error: " << context->errstr << "\n";
                redisFree(context);
            }
            else
            {
                std::cerr << "Can't allocate redis context\n";
            }
            throw std::runtime_error("Failed to connect to Redis");
        }
    }

    void InitData()
    {
        for (int i = 0; i < ARRER_NUM; i++)
        {
            data.id[i] = i;
            data.f_data[i] = i * 1.1;
            memset(&data.c_data, 0x0, sizeof(data.c_data));
        }
    }

    ~TestRedis()
    {
        if (context)
        {
            redisFree(context);
        }
    }

    void TestBinSet(int num_operations)
    {
        auto start = std::chrono::high_resolution_clock::now();
        char *buff = new char[sizeof(TestData)];

        for (int i = 0; i < num_operations; i += PIPELINE_SIZE)
        {
            // Start the pipeline
            for (int j = 0; j < PIPELINE_SIZE && (i + j) < num_operations; ++j)
            {
                std::string key = bin_base_key + std::to_string(i + j);
                strncpy(data.c_data, key.c_str(), sizeof(data.c_data));
                memcpy(buff, &data, sizeof(TestData));
                redisAppendCommand(context, "SET %s %b", key.c_str(), buff, sizeof(TestData));
            }

            // Get replies for the commands in the pipeline
            for (int j = 0; j < PIPELINE_SIZE && (i + j) < num_operations; ++j)
            {
                redisReply *reply;
                redisGetReply(context, (void **)&reply);
                freeReplyObject(reply);
            }
        }

        delete[] buff;
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        auto us = duration.count();
        double sec = (double)us / 1000000;
        std::cout << "BinSet " << num_operations << " items took: " << us / 1000 << " ms. avg: " << us / num_operations
                  << " us. " << num_operations / sec << " per second. " << std::endl;
    }

    void TestBinGet(int num_operations)
    {
        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < num_operations; i += PIPELINE_SIZE)
        {
            // Start the pipeline
            for (int j = 0; j < PIPELINE_SIZE && (i + j) < num_operations; ++j)
            {
                std::string key = bin_base_key + std::to_string(i + j);
                redisAppendCommand(context, "GET %s", key.c_str());
            }

            // Get replies for the commands in the pipeline
            for (int j = 0; j < PIPELINE_SIZE && (i + j) < num_operations; ++j)
            {
                redisReply *reply;
                redisGetReply(context, (void **)&reply);

                if (reply != nullptr && reply->type == REDIS_REPLY_STRING)
                {
                    memcpy(&data, reply->str, sizeof(TestData));
                }
                freeReplyObject(reply);
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        auto us = duration.count();
        double sec = (double)us / 1000000;
        std::cout << "BinGet " << num_operations << " items took: " << us / 1000 << " ms. avg: " << us / num_operations
                  << " us. " << num_operations / sec << " per second. " << std::endl;
    }

  private:
    redisContext *context;
    TestData data;
};

enum OPER_TYPE
{
    SET = 0x01,
    GET = 0x02,
    BOTH = 0x03
};

int main(int argc, char **argv)
{
    int set_flag = OPER_TYPE::BOTH;
    if (argc > 1)
    {
        if (strcmp(argv[1], "get") == 0)
        {
            set_flag = OPER_TYPE::GET;
        }
        else
            set_flag = OPER_TYPE::SET;
    }

    if (argc == 3)
    {
        set_flag = OPER_TYPE::BOTH;
        PIPELINE_SIZE = std::atoi(argv[2]);
    }

    try
    {
        TestRedis tester("127.0.0.1", 6379);
        // TestRedis tester("/var/run/redis/redis.sock");
        tester.InitData();

        int num_operations = 1000 * 200;

        if (set_flag & OPER_TYPE::SET)
        {
            tester.TestBinSet(num_operations);
        }
        if (set_flag & OPER_TYPE::GET)
        {
            tester.TestBinGet(num_operations);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
