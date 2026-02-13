/**
 * @file example.cpp
 * @brief Redis连接池使用示例
 * @version 1.0
 */

#include "redis_pool.h"
#include <chrono>
#include <iostream>
#include <thread>

using namespace redis_pool;

// 配置连接池
RedisConfig config;

void basic_usage_example()
{
    std::cout << "\n========== 基本使用示例 ==========\n";

    // 配置连接池
    config.pool_size = 10;    // 10个连接
    config.timeout_ms = 3000; // 3秒超时

    // 初始化连接池
    if (!RedisPool::GetInstance().Init(config))
    {
        std::cerr << "连接池初始化失败\n";
        return;
    }

    // 使用RAII方式获取连接
    {
        RedisConnectionGuard guard;
        if (!guard.IsValid())
        {
            std::cerr << "获取连接失败\n";
            return;
        }

        // 执行SET命令
        redisReply *reply = guard.Command("SET mykey hello_redis");
        if (reply)
        {
            std::cout << "SET 结果: " << reply->str << "\n";
            freeReplyObject(reply);
        }

        // 执行GET命令
        reply = guard.Command("GET mykey");
        if (reply && reply->type == REDIS_REPLY_STRING)
        {
            std::cout << "GET 结果: " << reply->str << "\n";
        }
        if (reply)
            freeReplyObject(reply);

        // 删除key
        reply = guard.Command("DEL mykey");
        if (reply)
            freeReplyObject(reply);
    } // 连接自动归还到池中

    // 销毁连接池
    RedisPool::GetInstance().Destroy();
}

void multithread_example()
{
    std::cout << "\n========== 多线程使用示例 ==========\n";

    config.pool_size = 20;

    if (!RedisPool::GetInstance().Init(config))
    {
        std::cerr << "连接池初始化失败\n";
        return;
    }

    const int num_threads = 10;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([i]() {
            // 每个线程执行一些操作
            for (int j = 0; j < 1000; ++j)
            {
                RedisConnectionGuard guard(5000); // 5秒超时
                if (!guard.IsValid())
                {
                    std::cerr << "线程 " << i << " 获取连接失败\n";
                    continue;
                }

                std::string key = "thread_" + std::to_string(i) + "_counter";

                // INCR操作
                redisReply *reply = guard.Command("INCR %s", key.c_str());
                if (reply)
                {
                    if (i == 0 && j % 100 == 0)
                        std::cout << "线程 " << i << " 第 " << j << " 次: " << key << " = " << reply->integer << "\n";
                    freeReplyObject(reply);
                }
                else
                {
                    std::cerr << "线程 " << i << " 第 " << j << " 次INCR失败\n";
                }
                if (i == 0 && j % 100 == 0)
                {
                    std::cout << RedisPool::GetInstance().GetAvailableCount() << "\n";
                }
                // 小延迟模拟工作
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            RedisConnectionGuard guard(5000); // 5秒超时
            std::string key = "thread_" + std::to_string(i) + "_counter";
            guard.Command("DEL %s", key.c_str());
        });
    }

    for (auto &t : threads)
    {
        t.join();
    }

    RedisPool::GetInstance().Destroy();
}

void pool_stats_example()
{
    std::cout << "\n========== 连接池统计信息示例 ==========\n";

    config.pool_size = 5;

    RedisPool::GetInstance().Init(config);

    std::cout << "总连接数: " << RedisPool::GetInstance().GetTotalCount() << "\n";
    std::cout << "可用连接数: " << RedisPool::GetInstance().GetAvailableCount() << "\n";

    // 获取3个连接
    auto conn1 = RedisPool::GetInstance().GetConnection();
    auto conn2 = RedisPool::GetInstance().GetConnection();
    auto conn3 = RedisPool::GetInstance().GetConnection();

    std::cout << "获取3个连接后，可用连接数: " << RedisPool::GetInstance().GetAvailableCount() << "\n";

    // 归还2个连接
    RedisPool::GetInstance().ReturnConnection(conn1);
    RedisPool::GetInstance().ReturnConnection(conn2);

    std::cout << "归还2个连接后，可用连接数: " << RedisPool::GetInstance().GetAvailableCount() << "\n";

    // 归还最后一个
    RedisPool::GetInstance().ReturnConnection(conn3);
    std::cout << "归还所有连接后，可用连接数: " << RedisPool::GetInstance().GetAvailableCount() << "\n";

    RedisPool::GetInstance().Destroy();
}

void advanced_usage_example()
{
    std::cout << "\n========== 高级使用示例 ==========\n";

    config.pool_size = 3;

    RedisPool::GetInstance().Init(config);

    // 使用管道（批量操作）
    {
        RedisConnectionGuard guard;
        if (guard.IsValid())
        {
            redisContext *ctx = guard.GetContext();

            // 使用管道发送多个命令
            redisAppendCommand(ctx, "SET pipeline_key1 value1");
            redisAppendCommand(ctx, "SET pipeline_key2 value2");
            redisAppendCommand(ctx, "SET pipeline_key3 value3");
            redisAppendCommand(ctx, "GET pipeline_key1");
            redisAppendCommand(ctx, "GET pipeline_key2");
            redisAppendCommand(ctx, "GET pipeline_key3");

            // 获取所有回复
            redisReply *reply;
            for (int i = 0; i < 6; ++i)
            {
                redisGetReply(ctx, (void **)&reply);
                if (reply)
                {
                    if (i < 3)
                    {
                        std::cout << "SET操作结果: " << (reply->str ? reply->str : "OK") << "\n";
                    }
                    else
                    {
                        std::cout << "GET操作结果: " << (reply->str ? reply->str : "nil") << "\n";
                    }
                    freeReplyObject(reply);
                }
            }

            // 清理
            guard.Command("DEL pipeline_key1 pipeline_key2 pipeline_key3");
        }
    }

    // MSET / MGET 示例（每次操作1000个）
    {
        RedisConnectionGuard guard;
        if (guard.IsValid())
        {
            const int batch_size = 1000;
            std::vector<std::string> keys;
            std::set<std::string> keys_set;
            std::vector<std::vector<char>> values;
            keys.reserve(batch_size);
            values.reserve(batch_size);

            for (int i = 0; i < batch_size; ++i)
            {
                std::string key = "mset_key_" + std::to_string(i);
                std::string value = "mset_val_" + std::to_string(i);
                keys.push_back(key);
                keys_set.insert(key);
                values.emplace_back(value.begin(), value.end());
            }

            std::vector<RedisPipelineResult> set_results;
            if (guard.MSet(keys, values, set_results))
            {
                size_t success_count = 0;
                for (const auto &result : set_results)
                {
                    if (result.success)
                        ++success_count;
                }
                std::cout << "MSET完成: " << success_count << "/" << set_results.size() << "\n";
            }
            else
            {
                std::cerr << "MSET失败\n";
            }

            std::unordered_map<std::string, RedisPipelineResult> get_results;
            keys_set.insert("unknown");
            if (guard.MGet(keys_set, get_results))
            {
                std::cout << "MGET完成: " << get_results.size() << "\n";
                for (int i = 0; i < batch_size; i++)
                {
                    const std::string key = "mset_key_" + std::to_string(i);
                    auto it = get_results.find(key);
                    if (it != get_results.end() && it->second.success)
                    {
                        std::string value(it->second.data.begin(), it->second.data.end());
                        if (i % 100 == 0)
                            std::cout << "MGET示例: " << key << " = " << value << "\n";
                    }
                    else
                    {
                        std::cerr << "MGET失败" << key << std::endl;
                    }
                }
            }
            else
            {
                std::cerr << "MGET失败\n";
            }

            // 清理批量key
            std::string del_cmd = "DEL";
            del_cmd.reserve(4 + batch_size * 20);
            for (const auto &key : keys)
            {
                del_cmd.append(" ").append(key);
            }
            redisReply *reply = guard.Command(del_cmd.c_str());
            if (reply)
                freeReplyObject(reply);
        }
    }

    RedisPool::GetInstance().Destroy();
}

int main(int argc, char *argv[])
{
    std::cout << "Redis连接池使用示例\n";
    std::cout << "===================\n";
    std::cout << "确保Redis服务器在 127.0.0.1:6379 运行\n";

    config.unix_sock_path = "/home/wzq/code/dsf3.0/dsf-redis/redis.sock"; // 使用Unix Socket（如果需要）
    config.host = "127.0.0.1";
    config.port = 6380;
    config.username = "dsf";      // Redis 6.0+ ACL认证
    config.password = "Dsf_123!"; // 如果需要密码

    // 运行各个示例
    basic_usage_example();
    multithread_example();
    pool_stats_example();
    advanced_usage_example();

    std::cout << "\n所有示例运行完成!\n";
    return 0;
}
