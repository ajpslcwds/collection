#include "string.h"
#include <iostream>
#include <string>

using namespace std;

void test(char *buf)
{
    strcpy(buf, "hello");
}

int main()
{
    std::string str(10, '\0');
    test(const_cast<char *>(str.c_str()));
    std::cout << str << std::endl;
    return 0;
}