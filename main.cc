#include <atomic>
#include <future>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <string.h>
#include <string>
#include <thread>
using namespace std;

class Temp
{
  public:
    std::atomic<int> a{1};
    std::atomic<int> b = 1;
    std::atomic<bool> bTemp = true;
};

void func()
{
    std::cout << "func_begin!" << std::endl;
    auto ft = std::async(std::launch::async, []() {
        std::cout << "async_begin!" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "async_end!" << std::endl;
    });
    std::cout << "func_end!" << std::endl;
}
int main(int argc, char *argv[])

{
    std::shared_ptr<Temp> temp = std::make_shared<Temp>();
    std::shared_ptr<Temp> temp2 = nullptr;

    temp2 = temp;
    if (temp2 == temp)
    {
        printf("temp2 == temp\n");
    }
    else
    {
        printf("temp2 != temp\n");
    }
    return 0;
}
