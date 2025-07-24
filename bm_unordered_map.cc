// g++ example.cpp -lbenchmark -pthread -O3 -o example

#include <benchmark/benchmark.h>
#include <string>
#include <unordered_map>
#include <vector>

struct RedisPiplineResult
{
    std::vector<char> data;
    bool success;
    uint32_t errCode;
};

// 测试函数1：使用 push_back 填充向量
void BM_assign(benchmark::State &state)
{
    std::string key = "key_";
    for (auto _ : state)
    {
        std::unordered_map<std::string, RedisPiplineResult> dataMap;
        dataMap.reserve(state.range(0)); // 提前分配空间
        for (int i = 0; i < state.range(0); ++i)
        {
            dataMap[key + std::to_string(i)] = {{}, false, 101};
        }
        benchmark::DoNotOptimize(dataMap);
    }
}

// 测试函数2：使用 reserve + push_back 填充向量
void BM_emplace(benchmark::State &state)
{
    std::string key = "key_";
    for (auto _ : state)
    {
        std::unordered_map<std::string, RedisPiplineResult> dataMap;
        dataMap.reserve(state.range(0)); // 提前分配空间
        for (int i = 0; i < state.range(0); ++i)
        {
            RedisPiplineResult r{{}, false, 101};
            dataMap.emplace(key + std::to_string(i), std::move(r));
        }
        benchmark::DoNotOptimize(dataMap);
    }
}

// 注册基准测试，并设置参数范围
BENCHMARK(BM_assign)->Range(8, 8 << 10); // 测试 8 到 8*2^10 的输入规模
BENCHMARK(BM_emplace)->Range(8, 8 << 10);
// 运行基准测试
BENCHMARK_MAIN();