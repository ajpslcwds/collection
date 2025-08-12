// g++ example.cpp -lbenchmark -pthread -O3 -o example

#include <benchmark/benchmark.h>
#include <string>
#include <unordered_map>
#include <vector>

// 测试函数1：使用 push_back 填充向量
void BM_yu(benchmark::State &state)
{
    for (auto _ : state)
    {
        for (int i = 0; i < state.range(0); ++i)
        {
            auto yu = i % 101;
        }
    }
}

// 测试函数2：使用 reserve + push_back 填充向量
void BM_chu(benchmark::State &state)
{
    for (auto _ : state)
    {
        for (int i = 0; i < state.range(0); ++i)
        {
            auto yu = i - i / 101 * 101;
        }
    }
}

// 注册基准测试，并设置参数范围
BENCHMARK(BM_yu)->Range(8, 8 << 10); // 测试 8 到 8*2^10 的输入规模
BENCHMARK(BM_chu)->Range(8, 8 << 10);
// 运行基准测试
BENCHMARK_MAIN();