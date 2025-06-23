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

// int main()
// {
//     std::string str = "3.402823e+38";
//     float f = std::stof(str);
//     printf("%f\n", f);   // 340282306073709652508363335590014353408.000000
//     printf("%e\n", f);   // 3.40282e+38
//     printf("%g\n", f);   // 3.40282e+38
//     printf("%.7g\n", f); // 3.402823e+38

//     std::cout << "---------------------------------" << std::endl;
//     float ff = 3.402823e+38;
//     std::cout << std::scientific << ff << std::endl;
//     std::cout << std::to_string(ff) << std::endl; // 340282306073709652508363335590014353408.000000

//     return 0;
// }

std::string floatToAutoString(float value, int precision = 7)
{
    std::ostringstream oss;
    oss << std::defaultfloat << std::setprecision(precision);
    oss << value;
    return oss.str();
}

std::string floatToAutoString(double value, int precision = 17)
{
    std::ostringstream oss;
    oss << std::defaultfloat << std::setprecision(precision);
    oss << value;
    return oss.str();
}

int main()
{
    {
        double d = -1.7976931348623157e+308;
        std::cout << d << std::endl;
        std::cout << std::to_string(d) << std::endl;
        printf("%f\n", d);
        printf("%e\n", d);
        printf("%g\n", d);
        printf("%.7g\n", d);
        std::cout << "---------------------------------" << std::endl;
    }

    {
        std::vector<float> vec = {0.0, 123.456, 0.000000123, 3.402823e+38, 1.175495e-38, -3.402823e+38, -1.175495e-38};
        for (auto &f : vec)
            std::cout << "f = " << floatToAutoString(f) << std::endl;
        std::cout << "---------------------------------" << std::endl;
    }

    {
        std::vector<double> vec = {0.0,
                                   123.456,
                                   0.000000123,
                                   3.402823e+38,
                                   1.175495e-38,
                                   2.2250738585072014e-308,
                                   1.7976931348623157e+308,
                                   -2.2250738585072014e-308,
                                   -1.7976931348623157e+308};
        for (auto &d : vec)
            std::cout << "d = " << floatToAutoString(d) << std::endl;

        std::cout << "---------------------------------" << std::endl;
    }

    {
        double x = 1.0 / 0.0;
        std::cout << "std::cout: " << x << std::endl;
        std::string s = std::to_string(x);
        std::cout << "std::to_string: " << s << std::endl;
        std::cout << "---------------------------------" << std::endl;
    }

    return 0;
}