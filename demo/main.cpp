#include <iostream>

#include "dsflog.h"
#include <thread>
int test() {
  LOG_DEBUG("value=%d",0);
  LOG_INFO("value=%d", 1);
  LOG_WARN("value=%d", 2);
  LOG_ERROR("failed: %s", "something wrong");
  LOG_DEBUG("value=%d",0);
  LOG_INFO("value=%d", 1);
  LOG_WARN("value=%d", 2);
  LOG_ERROR("failed: %s", "something wrong");

  return 0;
}

int main() {
  LOG_INIT("/home/wzq/code/collection/conf/dsflog.conf");
  for (int i =0;i<100;i++)
  {
    test();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  LOG_SHUTDOWN();
  return 0;
}