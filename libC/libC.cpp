
#include "libC.h"
#include "libB/libB.h"
#include <iostream>

void Func_C()
{
    std::cout << "This is Func_C from libC" << std::endl;
    Func_B();
}
