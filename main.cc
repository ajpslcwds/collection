#include "json.hpp"
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
#include <vector>

using namespace std;

int main()
{
    std::string str = "3.402823e+38";
    float f = std::stof(str);
    printf("%f\n", f);   // 340282306073709652508363335590014353408.000000
    printf("%g\n", f);   // 3.40282e+38
    printf("%.7g\n", f); // 3.402823e+38

    float ff = 3.402823e+38;
    std::cout << std::to_string(ff) << std::endl; // 340282306073709652508363335590014353408.000000

    // double d = std::stod(str);
    // long double ld = std::stold(str);
    // std::cout << "float:      " << f << std::endl;   // 3.40282e+38
    // std::cout << "double:      " << d << std::endl;  // 3.40282e+38
    // std::cout << "long double: " << ld << std::endl; // 3.40282e+38

    std::string strr;
    strr.reserve(0);
    cout << strr << endl;

    return 0;
}