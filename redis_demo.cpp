// g++ redis_demo.cpp -o redis_demo -lhiredis -pthread -g

#include <chrono>
#include <hiredis/hiredis.h>
#include <iostream>
#include <sstream>
#include <string.h>

const std::string base_key = "this_is_a_test_key_";
const std::string bin_base_key = "this_is_a_test_bin_key_";

struct TestData
{
    int id[20];
    float f_data[20];
    char c_data[64]; // save key
};

class TestRedis
{
  public:
    TestRedis(const std::string &host, int port)
    {
        for (int i = 0; i < 20; i++)
        {
            data.id[i] = i;
            data.f_data[i] = i * 1.0;
            memset(&data.c_data, 0x0, sizeof(data.c_data));
        }

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

    ~TestRedis()
    {
        if (context)
        {
            redisFree(context);
        }
    }

    // 添加序列化方法
    std::string serialize(const TestData &data)
    {
        std::ostringstream oss;
        for (int i = 0; i < 20; ++i)
        {
            oss << data.id[i] << "," << data.f_data[i] << ",";
        }
        oss << data.c_data; // 追加字符数组
        return oss.str();
    }

    // 添加反序列化方法
    TestData deserialize(const std::string &str)
    {
        TestData data;
        std::istringstream iss(str);
        std::string token;

        for (int i = 0; i < 20; ++i)
        {
            std::getline(iss, token, ',');
            data.id[i] = std::stoi(token);
            std::getline(iss, token, ',');
            data.f_data[i] = std::stof(token);
        }
        iss.getline(data.c_data, sizeof(data.c_data)); // 读取字符数组
        return data;
    }

    void TestSet(int num_operations)
    {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < num_operations; ++i)
        {

            std::string key = base_key + std::to_string(i);
            strncpy(data.c_data, key.c_str(), sizeof(data.c_data));
            std::string value = serialize(data); // 序列化数据
            redisReply *reply = (redisReply *)redisCommand(context, "SET %s %s", key.c_str(), value.c_str());
            freeReplyObject(reply);
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "Set " << num_operations << " items took: " << duration.count() / 1000
                  << " ms. avg: " << duration.count() / num_operations << " us." << std::endl;
    }

    void TestGet(int num_operations)
    {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < num_operations; ++i)
        {
            std::string key = base_key + std::to_string(i);
            redisReply *reply = (redisReply *)redisCommand(context, "GET %s", key.c_str());
            if (reply != nullptr)
            {
                TestData retrieved_data = deserialize(reply->str); // 反序列化数据
                // if (100 == i)
                // {
                //     std::cout << "Retrieved data for key: " << key << ", value: " << retrieved_data.c_data <<
                //     std::endl;
                // }
                freeReplyObject(reply);
                // 如果需要可以对 retrieved_data 进行后续操作
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "Get " << num_operations << " items took: " << duration.count() / 1000
                  << " ms. avg: " << duration.count() / num_operations << " us." << std::endl;
    }

    void TestBinSet(int num_operations)
    {
        auto start = std::chrono::high_resolution_clock::now();
        char *buff = new char[sizeof(TestData)];
        for (int i = 0; i < num_operations; ++i)
        {

            std::string key = bin_base_key + std::to_string(i);
            strncpy(data.c_data, key.c_str(), sizeof(data.c_data));
            memcpy(buff, &data, sizeof(TestData));
            std::string value = serialize(data); // 序列化数据
            redisReply *reply = (redisReply *)redisCommand(context, "SET %s %b", key.c_str(), buff, sizeof(TestData));
            freeReplyObject(reply);
        }
        delete[] buff;
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "BinSet " << num_operations << " items took: " << duration.count() / 1000
                  << " ms. avg: " << duration.count() / num_operations << " us." << std::endl;
    }

    void TestBinGet(int num_operations)
    {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < num_operations; ++i)
        {
            std::string key = bin_base_key + std::to_string(i);
            // 从 Redis 获取二进制数据
            redisReply *reply = (redisReply *)redisCommand(context, "GET %s", key.c_str());

            if (reply != nullptr && reply->type == REDIS_REPLY_STRING)
            {
                memcpy(&data, reply->str, sizeof(TestData));
                // TestData *p_data = (TestData *)reply->str;
                // if (100 == i)
                // {
                //     std::cout << "Retrieved data for key: " << key << ", value: " << p_data->c_data << std::endl;
                // }
            }
            freeReplyObject(reply); // 释放回复对象
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "BinGet " << num_operations << " items took: " << duration.count() / 1000
                  << " ms. avg: " << duration.count() / num_operations << " us." << std::endl;
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

    try
    {
        TestRedis tester("127.0.0.1", 6379);
        int num_operations = 1000 * 200;

        if (set_flag & OPER_TYPE::SET)
        {
            tester.TestSet(num_operations);
            tester.TestBinSet(num_operations);
            // tester.TestHSet(num_operations);
        }
        if (set_flag & OPER_TYPE::GET)
        {

            tester.TestGet(num_operations);
            tester.TestBinGet(num_operations);
            // tester.TestHGet(num_operations);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

/*
       # 直接操作hash ，超级慢
       void TestHSet(int num_operations)
       {
           auto start = std::chrono::high_resolution_clock::now();
           for (int i = 0; i < num_operations; ++i)
           {
               std::string key = "hash_key" + std::to_string(i);
               strncpy(data.c_data, key.c_str(), sizeof(data.c_data));

               // 使用 HMSET 将 TestData 存储为 Hash
               for (int j = 0; j < 20; ++j)
               {
                   std::string id_field = "id_" + std::to_string(j);
                   std::string fdata_field = "f_data_" + std::to_string(j);
                   redisCommand(context, "HSET %s %s %d", key.c_str(), id_field.c_str(), data.id[j]);
                   redisCommand(context, "HSET %s %s %f", key.c_str(), fdata_field.c_str(), data.f_data[j]);
               }

               // 存储字符数组 c_data
               redisCommand(context, "HSET %s c_data %s", key.c_str(), data.c_data);
           }
           auto end = std::chrono::high_resolution_clock::now();
           auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
           std::cout << "Set " << num_operations << " items took: " << duration.count() / 1000
                     << " ms. avg: " << duration.count() / num_operations << " us." << std::endl;
       }

       void TestHGet(int num_operations)
       {
           auto start = std::chrono::high_resolution_clock::now();
           for (int i = 0; i < num_operations; ++i)
           {
               std::string key = "hash_key" + std::to_string(i);

               // 使用 HGETALL 获取整个 Hash
               redisReply *reply = (redisReply *)redisCommand(context, "HGETALL %s", key.c_str());
               if (reply != nullptr && reply->elements > 0)
               {
                   for (size_t j = 0; j < reply->elements; j += 2)
                   {
                       std::string field(reply->element[j]->str);
                       std::string value(reply->element[j + 1]->str);

                       if (field.find("c_data") != std::string::npos)
                       {
                           strncpy(data.c_data, value.c_str(), sizeof(data.c_data));
                       }
                       else if (field.find("id_") != std::string::npos)
                       {
                           int index = std::stoi(field.substr(3)); // 提取索引
                           data.id[index] = std::stoi(value);
                       }
                       else if (field.find("f_data_") != std::string::npos)
                       {
                           int index = std::stoi(field.substr(7)); // 提取索引
                           data.f_data[index] = std::stof(value);
                       }
                   }
                   freeReplyObject(reply);
               }
           }
           auto end = std::chrono::high_resolution_clock::now();
           auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
           std::cout << "Get " << num_operations << " items took: " << duration.count() / 1000
                     << " ms. avg: " << duration.count() / num_operations << " us." << std::endl;
       }
   */