/**
 * @file redis_connection.cpp
 * @brief Redis连接类实现文件
 * @version 2.0
 */

#include "redis_connection.h"
#include <iostream>
#include <thread>

namespace redis_pool 
{

RedisConnection::RedisConnection(const RedisConfig& config)
    : config_(config), context_(nullptr), id_(-1), failure_count_(0) {
    ConnectInternal();
}

RedisConnection::~RedisConnection() {
    if (context_) {
        redisFree(context_);
        context_ = nullptr;
    }
}

bool RedisConnection::ConnectInternal() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 关闭旧连接
    if (context_) {
        redisFree(context_);
        context_ = nullptr;
    }

    // 设置超时
    struct timeval timeout;
    timeout.tv_sec = config_.timeout_ms / 1000;
    timeout.tv_usec = (config_.timeout_ms % 1000) * 1000;

    // 连接Redis (优先使用Unix Socket)
    if (!config_.unix_sock_path.empty()) {
        context_ = redisConnectUnixWithTimeout(config_.unix_sock_path.c_str(), timeout);
    } else {
        context_ = redisConnectWithTimeout(config_.host.c_str(), config_.port, timeout);
    }
    
    if (!context_ || context_->err) {
        if (context_) {
            std::cerr << "Redis连接错误: " << context_->errstr << std::endl;
            redisFree(context_);
            context_ = nullptr;
        } else {
            std::cerr << "Redis连接错误: 无法分配上下文" << std::endl;
        }
        failure_count_++;
        return false;
    }

    // 设置超时选项
    if (redisSetTimeout(context_, timeout) != REDIS_OK) {
        std::cerr << "设置超时失败: " << context_->errstr << std::endl;
        redisFree(context_);
        context_ = nullptr;
        failure_count_++;
        return false;
    }

    // 认证
    if (!Authenticate()) {
        redisFree(context_);
        context_ = nullptr;
        failure_count_++;
        return false;
    }

    // 选择数据库
    if (!SelectDatabase()) {
        redisFree(context_);
        context_ = nullptr;
        failure_count_++;
        return false;
    }

    last_used_ = std::chrono::steady_clock::now();
    failure_count_ = 0;
    return true;
}

bool RedisConnection::Authenticate() {
    if (config_.password.empty()) {
        return true; // 无需认证
    }

    redisReply* reply = nullptr;
    
    // Redis 6.0+ 使用 ACL 认证 (AUTH username password)
    if (!config_.username.empty()) {
        reply = (redisReply*)redisCommand(context_, "AUTH %s %s", 
                                          config_.username.c_str(), 
                                          config_.password.c_str());
    } else {
        // 旧版认证 (AUTH password)
        reply = (redisReply*)redisCommand(context_, "AUTH %s", config_.password.c_str());
    }

    if (!reply) {
        std::cerr << "Redis认证失败: 无响应" << std::endl;
        return false;
    }

    if (reply->type == REDIS_REPLY_ERROR) {
        std::cerr << "Redis认证失败: " << reply->str << std::endl;
        freeReplyObject(reply);
        return false;
    }

    freeReplyObject(reply);
    return true;
}

bool RedisConnection::SelectDatabase() {
    if (config_.db == 0) {
        return true; // 默认数据库，无需选择
    }

    redisReply* reply = (redisReply*)redisCommand(context_, "SELECT %d", config_.db);
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        if (reply) {
            std::cerr << "选择数据库失败: " << reply->str << std::endl;
            freeReplyObject(reply);
        } else {
            std::cerr << "选择数据库失败: 无响应" << std::endl;
        }
        return false;
    }
    
    freeReplyObject(reply);
    return true;
}

bool RedisConnection::IsValid() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!context_) {
        return false;
    }

    // 检查连接错误状态
    if (context_->err) {
        return false;
    }

    // 发送PING检查连接
    redisReply* reply = (redisReply*)redisCommand(context_, "PING");
    if (!reply || reply->type == REDIS_REPLY_ERROR) {
        if (reply) {
            freeReplyObject(reply);
        }
        return false;
    }

    freeReplyObject(reply);
    return true;
}

bool RedisConnection::Reconnect(int retry_times) {
    if (retry_times < 0) {
        retry_times = config_.max_retry_times;
    }

    for (int i = 0; i < retry_times; ++i) {
        if (i > 0) {
            std::cout << "第 " << i + 1 << " 次重连尝试 (连接ID: " << id_ << ")..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(config_.retry_interval_ms));
        }

        if (ConnectInternal()) {
            std::cout << "连接 " << id_ << " 重连成功" << std::endl;
            return true;
        }
    }

    std::cerr << "连接 " << id_ << " 重连失败，已尝试 " << retry_times << " 次" << std::endl;
    return false;
}

} // namespace redis_pool
