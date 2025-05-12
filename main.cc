#include <atomic>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <stdio.h>
#include <string.h>
#include <string>
#include <thread>

using namespace std;

std::mutex g_mutex;
std::condition_variable g_cv;
uint32_t FastSampledStringHash(const std::string &str)
{
    // return std::hash<std::string>{}(str); //20 times slower than below

    size_t len = str.size();
    if (len == 0)
        return 0;
    uint32_t hash = len; // default hash
    size_t step = len / 4;
    step = (step == 0 ? 1 : step);
    for (size_t i = 0; i < 4; ++i)
    {
        size_t idx = i * step;
        if (idx >= len)
            idx = len - 1;
        hash = (hash * 31) + static_cast<unsigned char>(str[idx]);
    }
    // last one
    hash = (hash * 31) + static_cast<unsigned char>(str[len - 1]);
    return hash;
}
int main(int argc, char *argv[])
{
    std::string str = "";
    int N = 10000;
    str.reserve(N);
    std::map<uint32_t, int> m;

    auto begin = std::chrono::system_clock::now();
    for (int i = 0; i < N; i++)
    {
        auto hash = FastSampledStringHash(str);
        m[hash % 101]++;
        // std::cout << str << "->" << FastSampledStringHash(str) << std::endl;
        str += i;
    }
    auto end = std::chrono::system_clock::now();
    std::cout << "time:" << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << std::endl;

    for (auto &item : m)
    {
        std::cout << item.first << ":" << item.second << std::endl;
    }
    return 0;
}
