#include <thread>

#include "../base/redis_interface.h"
int main() {
  RedisClient redis;

  if (!redis.Connect("127.0.0.1", 6380)) {
    std::cerr << "Redis connect failed\n";
    return 1;
  }

  while (10) {
    std::cout << "SET key1 value1 => " << redis.Command("SET key1 value1")
              << std::endl;
    std::cout << "GET key1 => " << redis.Command("GET key1") << std::endl;

    std::vector<std::string> cmds = {"SET key2 123", "INCR key2", "GET key3"};
    std::vector<std::string> replies;

    if (redis.Pipeline(cmds, &replies)) {
      std::cout << "Pipeline results:\n";
      for (auto& r : replies) {
        std::cout << "  -> " << r << std::endl;
      }
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  redis.Disconnect();
  return 0;
}
