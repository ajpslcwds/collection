#include <chrono>
#include <iostream>
#include <libmemcached/memcached.h>

int main()
{
    // 创建 Memcached 实例
    memcached_st *memc;
    memcached_return rc;
    memcached_server_st *servers;

    // 初始化 Memcached
    memc = memcached_create(NULL);
    servers = memcached_server_list_append(NULL, "localhost", 11211, &rc);
    rc = memcached_server_push(memc, servers);
    memcached_server_list_free(servers);

    if (rc != MEMCACHED_SUCCESS)
    {
        std::cerr << "Couldn't connect to Memcached server: " << memcached_strerror(memc, rc) << std::endl;
        return 1;
    }

    const int num_items = 1000 * 100;
    const std::string base_key = "my_key_";
    const std::string value = "value";

    {
        // 记录 set 操作开始时间
        auto start_set = std::chrono::high_resolution_clock::now();

        // 设置 10万条数据
        for (int i = 0; i < num_items; ++i)
        {
            std::string key = base_key + std::to_string(i);
            rc = memcached_set(memc, key.c_str(), key.length(), value.c_str(), value.length(), (time_t)0, (uint32_t)0);

            if (rc != MEMCACHED_SUCCESS)
            {
                std::cerr << "Couldn't store data for key: " << key << ", error: " << memcached_strerror(memc, rc)
                          << std::endl;
            }
        }

        // 记录 set 操作结束时间
        auto end_set = std::chrono::high_resolution_clock::now();
        auto duration_set = std::chrono::duration_cast<std::chrono::microseconds>(end_set - start_set);
        std::cout << "Set 100,000 items took: " << duration_set.count() << " microseconds." << std::endl;
    }

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
                free(retrieved_value);
            }
        }

        // 记录 get 操作结束时间
        auto end_get = std::chrono::high_resolution_clock::now();
        auto duration_get = std::chrono::duration_cast<std::chrono::microseconds>(end_get - start_get);
        std::cout << "Get 100,000 items took: " << duration_get.count() << " microseconds. failed cnt: " << failed_cnt
                  << std::endl;
    }

    // 清理并关闭连接
    memcached_free(memc);

    return 0;
}