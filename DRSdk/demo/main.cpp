#include "drsdk.h"
#include <iostream>

void TestGetId()
{
    uint32_t value = 0;
    auto res = drsdk::dr_get_id("wzq.test1", &value);
    LOG_INFO << "dr_get_id:res = " << res << ",value=" << value << std::endl;
}
void TestReadData()
{
    std::string value = "";
    auto res = drsdk::dr_read_data("wzq.test1", &value);
    LOG_INFO << "dr_read_data:res = " << res << ",value=" << value << std::endl;
}

void TestControlData()
{
    std::string value = "hello world";
    auto res = drsdk::dr_control_data("wzq.test1", value, 0);
    LOG_INFO << "dr_control_data:res = " << res << std::endl;
}

void TestAsyncRegTag()
{
    auto func1 = [](const std::string &value) { LOG_INFO << "fun_1 value=" << value << std::endl; };
    auto res1 = drsdk::dr_async_reg_tag("wzq.test1", func1);
    LOG_INFO << "dr_async_reg_tag :res = " << res1 << std::endl;

    auto func2 = [](const std::string &value) { LOG_INFO << "fun_2 value=" << value << std::endl; };
    auto res2 = drsdk::dr_async_reg_tag("wzq.test2", func2);
    LOG_INFO << "dr_async_reg_tag :res = " << res2 << std::endl;
}

int main()
{
    // TestAsyncRegTag();
    while (1)
    {
        TestGetId();
        // TestReadData();
        // TestControlData();
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return 0;
}
