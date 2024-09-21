#include <future>
#include <iostream>

std::future<int> asyncTask(bool flag)
{
    if (flag)
        return std::async([]() { return 8; });
    else
        return std::future<int>();
}
int main()
{
    auto res = asyncTask(0);
    if (res.valid())

    {
        std::cout << res.get() << std::endl;
    }
    else
    {
        std::cout << "invalid" << std::endl;
    }

    return 0;
}