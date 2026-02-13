/**
 * @file redis_pool.h
 * @brief Redis连接池类头文件
 * @version 2.0
 */

#ifndef REDIS_POOL_H
#define REDIS_POOL_H

#include "redis_connection.h"
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace redis_pool
{

enum RedisErrorCode : uint32_t
{
    OK = 0,
    CONTEXT_ERROR = 1,
    GET_REPLY_FALED = 2,
    GET_FAILED = 3,
    SET_FAILED = 4,
};

struct RedisPipelineResult
{
    std::vector<char> data;
    bool success;
    uint32_t error_code;
};

/**
 * @brief Redis连接池类
 *
 * 单例模式，管理Redis连接池，提供连接获取、归还、自动重连等功能
 */
class RedisPool
{
  public:
    using ConnectionPtr = std::shared_ptr<RedisConnection>;

    /**
     * @brief 获取单例实例
     * @return RedisPool引用
     */
    static RedisPool &GetInstance();

    /**
     * @brief 初始化连接池
     * @param config Redis配置
     * @return true=成功 false=失败
     */
    bool Init(const RedisConfig &config);

    /**
     * @brief 获取连接
     * @param timeout_ms 超时时间(毫秒)，-1表示无限等待
     * @return 连接指针，失败返回nullptr
     */
    ConnectionPtr GetConnection(int timeout_ms = -1);

    /**
     * @brief 归还连接
     * @param conn 连接指针
     */
    void ReturnConnection(ConnectionPtr conn);

    /**
     * @brief 获取可用连接数
     * @return 可用连接数
     */
    size_t GetAvailableCount();

    /**
     * @brief 获取总连接数
     * @return 总连接数
     */
    size_t GetTotalCount()
    {
        return config_.pool_size;
    }

    /**
     * @brief 检查连接池是否初始化
     * @return true=已初始化 false=未初始化
     */
    bool IsInitialized() const
    {
        return initialized_;
    }

    /**
     * @brief 销毁连接池
     */
    void Destroy();

    /**
     * @brief 禁止拷贝和赋值
     */
    RedisPool(const RedisPool &) = delete;
    RedisPool &operator=(const RedisPool &) = delete;

  private:
    /**
     * @brief 私有构造函数（单例模式）
     */
    RedisPool() = default;

    /**
     * @brief 析构函数
     */
    ~RedisPool();

    /**
     * @brief 创建连接池中的所有连接
     * @return true=成功 false=失败
     */
    bool CreateConnections();

    /**
     * @brief 健康检查线程函数
     */
    void HealthCheckThread();

    /**
     * @brief 检查并重连失效的连接
     * @param conn 连接指针
     * @return true=连接有效或重连成功 false=连接失效且重连失败
     */
    bool CheckAndReconnect(ConnectionPtr conn);

    RedisConfig config_;                              // 配置
    std::queue<ConnectionPtr> available_connections_; // 可用连接队列
    std::vector<ConnectionPtr> all_connections_;      // 所有连接列表
    std::mutex mutex_;                                // 互斥锁
    std::condition_variable cv_;                      // 条件变量
    std::atomic<bool> initialized_{false};            // 是否已初始化
    std::atomic<bool> stop_{false};                   // 停止标志
    std::thread health_check_thread_;                 // 健康检查线程
};

/**
 * @brief RAII风格的连接守卫类
 *
 * 自动获取和归还连接，确保连接正确归还到连接池
 */
class RedisConnectionGuard
{
  public:
    /**
     * @brief 构造函数，自动获取连接
     * @param timeout_ms 超时时间(毫秒)，<-1表示无限等待
     */
    explicit RedisConnectionGuard(int timeout_ms = 5);

    /**
     * @brief 析构函数，自动归还连接
     */
    ~RedisConnectionGuard();

    /**
     * @brief 获取原始redisContext
     * @return redisContext指针
     */
    redisContext *GetContext()
    {
        return conn_ ? conn_->GetContext() : nullptr;
    }

    /**
     * @brief 检查连接是否有效
     * @return true=有效 false=无效
     */
    bool IsValid()
    {
        return conn_ && conn_->IsValid();
    }

    /**
     * @brief 执行Redis命令（格式化字符串方式）
     * @param format 命令格式
     * @param ... 参数列表
     * @return redisReply指针，使用后需要调用freeReplyObject释放
     */
    redisReply *Command(const char *format, ...);

    /**
     * @brief 执行Redis命令（va_list方式）
     * @param format 命令格式
     * @param ap 参数列表
     * @return redisReply指针，使用后需要调用freeReplyObject释放
     */
    redisReply *Vcommand(const char *format, va_list ap);

    /**
     * @brief 执行Redis命令（参数数组方式）
     * @param argc 参数个数
     * @param argv 参数数组
     * @param argvlen 参数长度数组
     * @return redisReply指针，使用后需要调用freeReplyObject释放
     */
    redisReply *CommandArgv(int argc, const char **argv, const size_t *argvlen);

    /**
     * @brief 追加命令到pipeline（不等待结果）
     * @param format 命令格式
     * @param ... 参数列表
     * @return REDIS_OK=成功 REDIS_ERR=失败
     */
    int AppendCommand(const char *format, ...);

    /**
     * @brief 追加命令到pipeline（va_list方式）
     * @param format 命令格式
     * @param ap 参数列表
     * @return REDIS_OK=成功 REDIS_ERR=失败
     */
    int VAppendCommand(const char *format, va_list ap);

    /**
     * @brief 获取pipeline结果
     * @param reply 输出参数，返回redisReply指针
     * @return REDIS_OK=成功 REDIS_ERR=失败
     */
    int GetReply(redisReply **reply);

    /**
     * @brief 批量设置（使用pipeline）
     * @param keys 键数组
     * @param values 值数组（二进制）
     * @return RedisPipelineResult 结果数组，每个元素对应一个key的结果
     */
    bool MSet(const std::vector<std::string> &keys, const std::vector<std::vector<char>> &values,
              std::vector<RedisPipelineResult> &results);

    /**
     * @brief 批量获取（使用pipeline）
     * @param keys 键数组
     * @return RedisPipelineResult 结果数组，每个元素对应一个key的结果
     */
    bool MGet(const std::set<std::string> &keys, std::unordered_map<std::string, RedisPipelineResult> &dataMap);

    /**
     * @brief 禁止拷贝和赋值
     */
    RedisConnectionGuard(const RedisConnectionGuard &) = delete;
    RedisConnectionGuard &operator=(const RedisConnectionGuard &) = delete;

  private:
    RedisPool::ConnectionPtr conn_; // 连接指针
};

} // namespace redis_pool

#endif // REDIS_POOL_H
