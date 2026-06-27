/**
 * 单例模式 (Singleton)
 *
 * 意图：确保一个类只有一个实例，并提供一个全局访问点
 *
 * C++17 特性：
 * - std::call_once 保证线程安全的单次初始化
 * - inline 变量避免头文件重复定义
 */

#include <iostream>
#include <memory>
#include <mutex>
#include <string>

// ============================================
// 模式核心实现
// ============================================

// 方式1: 使用静态局部变量 (C++11+)
class Singleton {
 public:
  // 删除拷贝构造和赋值操作
  Singleton(const Singleton&) = delete;
  Singleton& operator=(const Singleton&) = delete;

  // 获取单例实例
  static Singleton* GetInstance() {
    static Singleton instance;
    return &instance;
  }

  // 示例功能
  void SetValue(const std::string& value) {
    value_ = value;
  }

  std::string GetValue() const {
    return value_;
  }

 private:
  Singleton() = default;
  ~Singleton() = default;

  std::string value_;
};

// 方式2: 使用 std::call_once (更明确的线程安全)
class ThreadSafeSingleton {
 public:
  ThreadSafeSingleton(const ThreadSafeSingleton&) = delete;
  ThreadSafeSingleton& operator=(const ThreadSafeSingleton&) = delete;

  static ThreadSafeSingleton* GetInstance() {
    static std::once_flag init_flag;
    static ThreadSafeSingleton* instance = nullptr;
    std::call_once(init_flag, []() {
      instance = new ThreadSafeSingleton();
    });
    return instance;
  }

  void DoSomething() {
    std::cout << "ThreadSafeSingleton doing something" << std::endl;
  }

 private:
  ThreadSafeSingleton() = default;
  ~ThreadSafeSingleton() = default;
};

// ============================================
// 使用示例
// ============================================

int main() {
  std::cout << "=== 单例模式 (Singleton) ===" << std::endl;

  // 示例1: 基本单例
  Singleton* s1 = Singleton::GetInstance();
  Singleton* s2 = Singleton::GetInstance();

  std::cout << "s1 和 s2 是同一个实例: "
            << (s1 == s2 ? "是" : "否") << std::endl;

  s1->SetValue("Hello Singleton");
  std::cout << "s2->GetValue(): " << s2->GetValue() << std::endl;

  // 示例2: 线程安全单例
  ThreadSafeSingleton* ts = ThreadSafeSingleton::GetInstance();
  ts->DoSomething();

  return 0;
}
