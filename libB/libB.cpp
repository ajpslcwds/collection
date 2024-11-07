
#include "libB.h"
#include "libA/libA.h"
#include <iostream>

void Func_B()
{
    std::cout << "This is Func_B from libB" << std::endl;
    Func_A();
}
