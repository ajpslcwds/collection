/**
 * @file redis_connection.h
 * @brief Redis连接类头文件
 * @version 2.0
 */

#ifndef REDIS_CONNECTION_H
#define REDIS_CONNECTION_H

#include <atomic>
#include <chrono>
#include <hiredis/hiredis.h>
#include <mutex>
#include <string>

namespace redis_pool
{

// Redis连接配置
struct RedisConfig
{
    std::string unix_sock_path; // Unix Socket路径(优先使用)
    std::string host = "127.0.0.1";
    int port = 6379;
    std::string username; // Redis 6.0+ ACL用户名
    std::string password;
    int db = 0;
    int pool_size = 20;           // 连接池大小
    int timeout_ms = 3000;        // 连接超时(毫秒)
    bool keep_alive = true;       // 是否保持连接
    int max_idle_time_ms = 60000; // 最大空闲时间(毫秒)
    int max_retry_times = 3;      // 最大重试次数
    int retry_interval_ms = 1000; // 重试间隔(毫秒)
};

/**
 * @brief Redis连接类
 *
 * 封装单个Redis连接，提供连接管理、自动重连、健康检查等功能
 */
class RedisConnection
{
  public:
    /**
     * @brief 构造函数
     * @param config Redis配置
     */
    explicit RedisConnection(const RedisConfig &config);

    /**
     * @brief 析构函数
     */
    ~RedisConnection();

    /**
     * @brief 检查连接是否有效
     * @return true=有效 false=无效
     */
    bool IsValid();

    /**
     * @brief 重新连接
     * @param retry_times 重试次数，默认使用配置中的值
     * @return true=成功 false=失败
     */
    bool Reconnect(int retry_times = -1);

    /**
     * @brief 获取原始redisContext
     * @return redisContext指针
     */
    redisContext *GetContext()
    {
        return context_;
    }

    /**
     * @brief 获取最后使用时间
     * @return 最后使用时间点
     */
    std::chrono::steady_clock::time_point GetLastUsedTime() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return last_used_;
    }

    /**
     * @brief 更新最后使用时间
     */
    void UpdateLastUsedTime()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        last_used_ = std::chrono::steady_clock::now();
    }

    /**
     * @brief 标记连接是否正在被使用
     */
    void SetInUse(bool in_use)
    {
        in_use_.store(in_use, std::memory_order_release);
    }

    /**
     * @brief 检查连接是否正在被使用
     */
    bool IsInUse() const
    {
        return in_use_.load(std::memory_order_acquire);
    }

    /**
     * @brief 获取连接ID
     * @return 连接ID
     */
    int GetId() const
    {
        return id_;
    }

    /**
     * @brief 设置连接ID
     * @param id 连接ID
     */
    void SetId(int id)
    {
        id_ = id;
    }

    /**
     * @brief 获取连接失败次数
     * @return 失败次数
     */
    int GetFailureCount() const
    {
        return failure_count_;
    }

    /**
     * @brief 重置失败计数
     */
    void ResetFailureCount()
    {
        failure_count_ = 0;
    }

    /**
     * @brief 禁止拷贝
     */
    RedisConnection(const RedisConnection &) = delete;
    RedisConnection &operator=(const RedisConnection &) = delete;

  private:
    /**
     * @brief 内部连接函数
     * @return true=成功 false=失败
     */
    bool ConnectInternal();

    /**
     * @brief 执行认证
     * @return true=成功 false=失败
     */
    bool Authenticate();

    /**
     * @brief 选择数据库
     * @return true=成功 false=失败
     */
    bool SelectDatabase();

    RedisConfig config_;                              // 配置
    redisContext *context_;                           // Redis上下文
    std::atomic<int> id_;                             // 连接ID
    std::atomic<int> failure_count_;                  // 失败计数
    std::atomic<bool> in_use_{false};                 // 是否正在被使用
    std::chrono::steady_clock::time_point last_used_; // 最后使用时间（由mutex_保护）
    mutable std::mutex mutex_;                        // 互斥锁
};

} // namespace redis_pool

#endif // REDIS_CONNECTION_H
