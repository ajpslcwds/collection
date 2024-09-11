#include "drsdk.h"
#include <iostream>

int main()
{
    uint32_t value = 0;

    auto res = drsdk::dr_get_id("wzq.test1", &value);
    std::cout << "dr_get_id:res = " << res << ",value=" << value << std::endl;

    return 0;
}
