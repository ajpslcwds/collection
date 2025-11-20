#include <sys/syscall.h>
#include <unistd.h>

#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "../base/object_pool.h"

int main() {
  // 创建 pool（5 个 std::string，每个默认构造）
  auto pool = std::make_shared<ObjectPool<std::string>>(5, std::string());

  {
    auto s1 = pool->GetObject();
    if (s1) {
      *s1 = "hello";
      std::cout << *s1 << '\n';
    }

    auto s2 = pool->GetObject();
    if (s2) {
      *s2 = "world";
      std::cout << *s2 << '\n';
    }
    // s1, s2 离开作用域时会被自动归还到 pool
  }

  // 再取出对象，验证可以重用
  auto s3 = pool->GetObject();
  if (s3) std::cout << "reused: '" << *s3 << "'\n";
  auto s4 = pool->GetObject();
  if (s4) std::cout << "reused: '" << *s4 << "'\n";
  return 0;
}
