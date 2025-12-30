
#include <sched.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

//  stress-ng --cpu 20 --cpu-load 90 --io 4
// top -Hp $(ps -ef|grep thread_p|grep -v sudo|grep -v grep |awk '{print $2}')

/**
 * 解析CPU集合字符串，并将其转换为整数向量
 * @param str 输入的CPU集合字符串，例如"0-3,5,7-9"
 * @param cpuset 用于存储解析结果的CPU ID向量指针
 */
void ParseCpuset(const std::string& str, std::vector<int>* cpuset) {
  std::vector<std::string> lines;  // 存储分割后的每个CPU范围或单个CPU
  std::stringstream ss(str);       // 使用字符串流处理输入字符串
  std::string l;                   // 临时存储每行内容
  // 按逗号分割字符串，获取每个CPU范围或单个CPU
  while (getline(ss, l, ',')) {
    lines.push_back(l);
  }
  for (auto line : lines) {
    std::stringstream ss(line);
    std::vector<std::string> range;
    while (getline(ss, l, '-')) {
      range.push_back(l);
    }
    if (range.size() == 1) {
      cpuset->push_back(std::stoi(range[0]));
    } else if (range.size() == 2) {
      for (int i = std::stoi(range[0]), e = std::stoi(range[1]); i <= e; i++) {
        cpuset->push_back(i);
      }
    } else {
      std::cout << "Parsing cpuset format error." << std::endl;
      exit(0);
    }
  }
}

void SetSchedAffinity(std::thread* thread, const std::vector<int>& cpus, const std::string& affinity, int cpu_id) {
  cpu_set_t set;
  CPU_ZERO(&set);

  if (cpus.size()) {
    if (!affinity.compare("range")) {
      for (const auto cpu : cpus) {
        CPU_SET(cpu, &set);
      }
      pthread_setaffinity_np(thread->native_handle(), sizeof(set), &set);
      std::cout << "thread " << thread->get_id() << " set range affinity" << std::endl;
    } else if (!affinity.compare("1to1")) {
      if (cpu_id == -1 || (uint32_t)cpu_id >= cpus.size()) {
        return;
      }
      CPU_SET(cpus[cpu_id], &set);
      pthread_setaffinity_np(thread->native_handle(), sizeof(set), &set);
      std::cout << "thread " << thread->get_id() << " set 1to1 affinity" << std::endl;
    }
  }
}

void SetSchedPolicy(std::thread* thread, std::string spolicy, int sched_priority, pid_t tid = -1) {
  struct sched_param sp;
  int policy;

  memset(reinterpret_cast<void*>(&sp), 0, sizeof(sp));
  sp.sched_priority = sched_priority;

  if (!spolicy.compare("SCHED_FIFO")) {
    policy = SCHED_FIFO;
    pthread_setschedparam(thread->native_handle(), policy, &sp);
    std::cout << "thread " << tid << " set sched_policy: " << spolicy << std::endl;
  } else if (!spolicy.compare("SCHED_RR")) {
    policy = SCHED_RR;
    pthread_setschedparam(thread->native_handle(), policy, &sp);
    std::cout << "thread " << tid << " set sched_policy: " << spolicy << std::endl;
  } else if (!spolicy.compare("SCHED_OTHER")) {
    setpriority(PRIO_PROCESS, tid, sched_priority);
    std::cout << "thread " << tid << " set sched_policy: " << spolicy << std::endl;
  }
}

std::atomic_bool gStop = {false};
uint32_t kInterval = 50;
void signalHandler(int signal) {
  if (signal == SIGINT) {
    std::cout << "\nCaught Ctrl+C (SIGINT). Exiting loop..." << std::endl;
    gStop.store(true);
  }
}

void threadFunc() {
  char name[16];
  pthread_getname_np(pthread_self(), name, sizeof(name));
  while (!gStop) {
    auto begin = std::chrono::high_resolution_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(kInterval));
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    if (duration > kInterval) {
      std::cout << "thread " << name << " sleep too long: " << duration << std::endl;
    } else {
      // std::cout << "thread " << std::this_thread::get_id() << " sleep: " << duration << std::endl;
    }
  }
}
int main() {
  std::signal(SIGINT, signalHandler);

  std::vector<std::thread> threads;
  {
    std::thread t1(threadFunc);
    pthread_setname_np(t1.native_handle(), "t1");

    {
      std::vector<int> cpu_set;
      ParseCpuset("0-7", &cpu_set);
      SetSchedAffinity(&t1, cpu_set, "1to1", 7);
      SetSchedPolicy(&t1, "SCHED_FIFO", 20);
    }
    threads.emplace_back(std::move(t1));
  }
  {
    std::thread t2(threadFunc);
    pthread_setname_np(t2.native_handle(), "t2");
    threads.emplace_back(std::move(t2));
  }

  {
    std::thread t3([]() {
      pid_t tid = syscall(SYS_gettid);
      setpriority(PRIO_PROCESS, tid, -3);
      threadFunc();
    });

    pthread_setname_np(t3.native_handle(), "t3");
    threads.emplace_back(std::move(t3));
  }
  // finish
  for (auto& t : threads) {
    if (t.joinable()) {
      t.join();
    }
  }

  return 0;
}