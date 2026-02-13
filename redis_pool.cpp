/**
 * @file redis_pool.cpp
 * @brief Redis连接池类实现文件
 * @version 2.0
 */

#include "redis_pool.h"
#include <algorithm>
#include <cstdarg>
#include <cstring>
#include <iostream>
#include <string>

namespace redis_pool
{

// ==================== RedisPool 实现 ====================

RedisPool &RedisPool::GetInstance()
{
    static RedisPool instance;
    return instance;
}

RedisPool::~RedisPool()
{
    Destroy();
}

bool RedisPool::Init(const RedisConfig &config)
{
    if (initialized_)
    {
        std::cerr << "连接池已经初始化" << std::endl;
        return false;
    }

    config_ = config;

    if (!CreateConnections())
    {
        return false;
    }

    initialized_ = true;
    stop_ = false;

    // 启动健康检查线程
    health_check_thread_ = std::thread(&RedisPool::HealthCheckThread, this);

    std::cout << "Redis连接池初始化成功，池大小: " << config_.pool_size << std::endl;
    return true;
}

bool RedisPool::CreateConnections()
{
    for (int i = 0; i < config_.pool_size; ++i)
    {
        auto conn = std::make_shared<RedisConnection>(config_);

        if (!conn->GetContext())
        {
            std::cerr << "创建连接 " << i << " 失败" << std::endl;
            return false;
        }

        conn->SetId(i);
        available_connections_.push(conn);
        all_connections_.push_back(conn);
    }

    return true;
}

RedisPool::ConnectionPtr RedisPool::GetConnection(int timeout_ms)
{
    if (!initialized_)
    {
        std::cerr << "连接池未初始化" << std::endl;
        return nullptr;
    }

    std::unique_lock<std::mutex> lock(mutex_);

    if (timeout_ms < 0)
    {
        // 无限等待
        cv_.wait(lock, [this]() { return !available_connections_.empty() || stop_; });
    }
    else
    {
        // 超时等待
        bool got_connection = cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms),
                                           [this]() { return !available_connections_.empty() || stop_; });

        if (!got_connection)
        {
            std::cerr << "获取连接超时" << std::endl;
            return nullptr;
        }
    }

    if (stop_)
    {
        return nullptr;
    }

    if (available_connections_.empty())
    {
        return nullptr;
    }

    auto conn = available_connections_.front();
    available_connections_.pop();
    conn->SetInUse(true);

    lock.unlock();

    // 检查连接是否有效，无效则重连
    if (!CheckAndReconnect(conn))
    {
        std::cerr << "连接 " << conn->GetId() << " 无效且重连失败" << std::endl;
        ReturnConnection(conn);
        return nullptr;
    }

    conn->UpdateLastUsedTime();
    return conn;
}

void RedisPool::ReturnConnection(ConnectionPtr conn)
{
    if (!conn)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    conn->SetInUse(false);
    conn->UpdateLastUsedTime();
    available_connections_.push(conn);
    cv_.notify_one();
}

size_t RedisPool::GetAvailableCount()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return available_connections_.size();
}

void RedisPool::Destroy()
{
    stop_ = true;
    cv_.notify_all();

    if (health_check_thread_.joinable())
    {
        health_check_thread_.join();
    }

    std::lock_guard<std::mutex> lock(mutex_);

    // 清空队列
    while (!available_connections_.empty())
    {
        available_connections_.pop();
    }

    all_connections_.clear();
    initialized_ = false;
}

bool RedisPool::CheckAndReconnect(ConnectionPtr conn)
{
    if (!conn)
    {
        return false;
    }

    // 检查连接是否有效
    if (conn->IsValid())
    {
        return true;
    }

    // 连接无效，尝试重连
    std::cout << "连接 " << conn->GetId() << " 无效，尝试重连..." << std::endl;

    if (conn->Reconnect())
    {
        std::cout << "连接 " << conn->GetId() << " 重连成功" << std::endl;
        return true;
    }

    std::cerr << "连接 " << conn->GetId() << " 重连失败" << std::endl;
    return false;
}

void RedisPool::HealthCheckThread()
{
    pthread_setname_np(pthread_self(), "redis_check");
    while (!stop_)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        if (stop_)
        {
            break;
        }

        // 从 available_connections_ 中取出空闲过久的连接，检查完再放回
        // 这样健康检查线程和业务线程操作同一个队列，不会并发操作同一个 redisContext*
        std::vector<ConnectionPtr> to_check;
        auto now = std::chrono::steady_clock::now();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            std::queue<ConnectionPtr> remaining;
            while (!available_connections_.empty())
            {
                auto conn = available_connections_.front();
                available_connections_.pop();
                auto idle_ms =
                    std::chrono::duration_cast<std::chrono::milliseconds>(now - conn->GetLastUsedTime()).count();
                if (idle_ms > 10000)
                {
                    to_check.push_back(conn);
                }
                else
                {
                    remaining.push(conn);
                }
            }
            available_connections_ = std::move(remaining);
        }

        // 在锁外执行网络 I/O，不阻塞业务线程获取其他连接
        for (auto &conn : to_check)
        {
            if (!conn->IsValid())
            {
                std::cout << "健康检查：连接 " << conn->GetId() << " 无效，尝试重连..." << std::endl;
                if (conn->Reconnect())
                {
                    std::cout << "健康检查：连接 " << conn->GetId() << " 重连成功" << std::endl;
                }
                else
                {
                    std::cerr << "健康检查：连接 " << conn->GetId() << " 重连失败" << std::endl;
                }
            }
            else
            {
                conn->UpdateLastUsedTime();
            }
        }

        // 检查完毕，放回队列并唤醒等待的业务线程
        if (!to_check.empty())
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (auto &conn : to_check)
            {
                available_connections_.push(conn);
            }
            cv_.notify_all();
        }
    }
}

// ==================== RedisConnectionGuard 实现 ====================

RedisConnectionGuard::RedisConnectionGuard(int timeout_ms)
{
    conn_ = RedisPool::GetInstance().GetConnection(timeout_ms);
}

RedisConnectionGuard::~RedisConnectionGuard()
{
    if (conn_)
    {
        RedisPool::GetInstance().ReturnConnection(conn_);
    }
}

redisReply *RedisConnectionGuard::Command(const char *format, ...)
{
    if (!conn_ || !conn_->GetContext())
    {
        return nullptr;
    }

    va_list ap;
    va_start(ap, format);
    redisReply *reply = (redisReply *)redisvCommand(conn_->GetContext(), format, ap);
    va_end(ap);

    return reply;
}

redisReply *RedisConnectionGuard::Vcommand(const char *format, va_list ap)
{
    if (!conn_ || !conn_->GetContext())
    {
        return nullptr;
    }

    return (redisReply *)redisvCommand(conn_->GetContext(), format, ap);
}

redisReply *RedisConnectionGuard::CommandArgv(int argc, const char **argv, const size_t *argvlen)
{
    if (!conn_ || !conn_->GetContext())
    {
        return nullptr;
    }

    return (redisReply *)redisCommandArgv(conn_->GetContext(), argc, argv, argvlen);
}

int RedisConnectionGuard::AppendCommand(const char *format, ...)
{
    if (!conn_ || !conn_->GetContext())
    {
        return REDIS_ERR;
    }

    va_list ap;
    va_start(ap, format);
    int result = redisvAppendCommand(conn_->GetContext(), format, ap);
    va_end(ap);

    return result;
}

int RedisConnectionGuard::VAppendCommand(const char *format, va_list ap)
{
    if (!conn_ || !conn_->GetContext())
    {
        return REDIS_ERR;
    }

    return redisvAppendCommand(conn_->GetContext(), format, ap);
}

int RedisConnectionGuard::GetReply(redisReply **reply)
{
    if (!conn_ || !conn_->GetContext())
    {
        return REDIS_ERR;
    }

    return redisGetReply(conn_->GetContext(), (void **)reply);
}

bool RedisConnectionGuard::MSet(const std::vector<std::string> &keys, const std::vector<std::vector<char>> &values,
                                std::vector<RedisPipelineResult> &results)
{
    results.clear();
    if (!conn_ || !conn_->GetContext() || keys.size() != values.size() || keys.empty())
    {
        return false;
    }

    results.reserve(keys.size());
    for (size_t i = 0; i < keys.size(); ++i)
    {
        const std::string &key = keys[i];
        const std::vector<char> &value = values[i];

        if (redisAppendCommand(conn_->GetContext(), "SET %s %b", key.c_str(), value.data(), value.size()) != REDIS_OK)
        {
            results.emplace_back(RedisPipelineResult{{}, false, RedisErrorCode::CONTEXT_ERROR});
        }
        else
        {
            results.emplace_back(RedisPipelineResult{{}, true, RedisErrorCode::OK});
        }
    }

    for (size_t i = 0; i < keys.size(); ++i)
    {
        if (!results[i].success)
        {
            continue; // AppendCommand 失败，没有对应的 reply 需要读取
        }

        redisReply *reply = nullptr;

        if (redisGetReply(conn_->GetContext(), (void **)&reply) != REDIS_OK || nullptr == reply ||
            nullptr == reply->str || reply->type != REDIS_REPLY_STATUS || std::string(reply->str) != "OK")
        {
            results[i].success = false;
            results[i].error_code = RedisErrorCode::GET_REPLY_FALED;
        }

        if (reply)
        {
            freeReplyObject(reply);
        }
    }

    return true;
}

bool RedisConnectionGuard::MGet(const std::set<std::string> &keys,
                                std::unordered_map<std::string, RedisPipelineResult> &res_data)
{
    res_data.clear();

    if (!conn_ || !conn_->GetContext())
    {
        return false;
    }

    for (const auto &key : keys)
    {
        if (redisAppendCommand(conn_->GetContext(), "GET %s", key.c_str()) != REDIS_OK)
        {
            res_data.emplace(key, RedisPipelineResult{{}, false, RedisErrorCode::CONTEXT_ERROR});
        }
        else
        {
            res_data.emplace(key, RedisPipelineResult{{}, true, RedisErrorCode::OK});
        }
    }
    for (const auto &key : keys)
    {
        if (!res_data[key].success)
        {
            continue; // AppendCommand 失败，没有对应的 reply 需要读取
        }

        redisReply *reply = nullptr;
        RedisPipelineResult result;
        if (redisGetReply(conn_->GetContext(), (void **)&reply) == REDIS_OK && reply)
        {
            if (reply->type == REDIS_REPLY_STRING && reply->str != nullptr)
            {
                res_data[key] = RedisPipelineResult{std::vector<char>(reply->str, reply->str + reply->len), true, 0};
            }
            else
            {
                res_data[key] = RedisPipelineResult{{}, false, RedisErrorCode::GET_REPLY_FALED};
            }
            freeReplyObject(reply);
        }
        else
        {
            res_data[key] = RedisPipelineResult{{}, false, RedisErrorCode::GET_REPLY_FALED};
        }
    }

    return true;
}

} // namespace redis_pool
