#include <chrono>
#include <iostream>
#include <libmemcached/memcached.h>
#include <thread>

const std::string base_key = "this_is_a_test_key_";
struct TestData
{
    int id[20];
    float f_data[20];
    char c_data[64];
};

class TestMemcached
{
  public:
    TestMemcached()
    {
        Init();
    }
    ~TestMemcached()
    {
        // 清理并关闭连接
        memcached_free(memc);
    }

    int Init()
    {
        for (int i = 0; i < 20; i++)
        {
            data.id[i] = i;
            data.f_data[i] = i * 1.0;
            memset(&data.c_data, 0x0, sizeof(data.c_data));
        }

        num_items = 1000 * 200;
        // 创建 Memcached 实例
        memc = memcached_create(NULL);
        servers = memcached_server_list_append(NULL, "localhost", 11211, &rc);
        rc = memcached_server_push(memc, servers);
        memcached_server_list_free(servers);

        if (rc != MEMCACHED_SUCCESS)
        {
            std::cerr << "Couldn't connect to Memcached server: " << memcached_strerror(memc, rc) << std::endl;
            return 1;
        }

        return 0;
    }

    void TestSet()
    {

        // 记录 set 操作开始时间
        auto start_set = std::chrono::high_resolution_clock::now();
        int failed_cnt = 0;
        // 设置 10万条数据
        char *buff = new char[sizeof(TestData) + 1];
        for (int i = 0; i < num_items; ++i)
        {
            std::string key = base_key + std::to_string(i);

            strncpy(data.c_data, key.c_str(), sizeof(data.c_data));
            memcpy(buff, &data, sizeof(data));

            int value_length = sizeof(data);

            rc = memcached_set(memc, key.c_str(), key.length(), buff, value_length, (time_t)0, (uint32_t)0);

            if (rc != MEMCACHED_SUCCESS)
            {
                // std::cerr << "Couldn't store data for key: " << key << ", error: " << memcached_strerror(memc, rc)
                //           << std::endl;
                ++failed_cnt;
            }

            // if (i % 1000 == 0)
            // {
            //     std::this_thread::sleep_for(std::chrono::milliseconds(1));
            // }
        }

        // 记录 set 操作结束时间
        auto end_set = std::chrono::high_resolution_clock::now();
        auto duration_set = std::chrono::duration_cast<std::chrono::microseconds>(end_set - start_set);
        std::cout << "Set " << num_items << " items took: " << duration_set.count()
                  << " microseconds. failed cnt: " << failed_cnt << std::endl;
    }

    void TestGet()
    { // 记录 get 操作开始时间
        auto start_get = std::chrono::high_resolution_clock::now();
        int failed_cnt = 0;
        // 获取 10万条数据
        for (int i = 0; i < num_items; ++i)
        {
            std::string key = base_key + std::to_string(i);
            size_t value_length;
            uint32_t flags;
            char *retrieved_value = memcached_get(memc, key.c_str(), key.length(), &value_length, &flags, &rc);

            if (rc != MEMCACHED_SUCCESS)
            {
                // std::cerr << "Couldn't retrieve data for key: " << key << ", error: " << memcached_strerror(memc, rc)
                //           << std::endl;
                ++failed_cnt;
            }
            else
            {
                // if (10 == i)
                // {
                //     TestData *p_data = (TestData *)retrieved_value;
                //     std::cout << "Retrieved data for key: " << key << ", value: " << p_data->c_data << std::endl;
                // }
                free(retrieved_value);
            }

            // if (i % 1000 == 0)
            // {
            //     std::this_thread::sleep_for(std::chrono::milliseconds(1));
            // }
        }

        // 记录 get 操作结束时间
        auto end_get = std::chrono::high_resolution_clock::now();
        auto duration_get = std::chrono::duration_cast<std::chrono::microseconds>(end_get - start_get);
        std::cout << "Get " << num_items << " items took: " << duration_get.count()
                  << " microseconds. failed cnt: " << failed_cnt << std::endl;
    }

  private:
    int num_items = 1;
    TestData data;
    memcached_st *memc = nullptr;
    memcached_return rc;
    memcached_server_st *servers;
};
enum OPER_TYPE
{
    SET = 0x01,
    GET = 0x02,
    BOTH = 0x03
};

int main(int argc, char **argv)
{
    // std::cout << "sizeof(TestData):" << sizeof(TestData) << std::endl;  // 20*4 + 20*4 +64 =224

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

    TestMemcached instance;
    if (set_flag & OPER_TYPE::SET)
        instance.TestSet();
    if (set_flag & OPER_TYPE::GET)
        instance.TestGet();

    // std::thread t1([&instance]() { instance.TestSet(); });
    // std::thread t2([&instance]() { instance.TestGet(); });

    // t1.join();
    // t2.join();

    return 0;
}
