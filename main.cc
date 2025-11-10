#include "data_types.h"
#include <atomic>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <thread>
#include <vector>
using namespace std;

int main()
{
    const std::string input = "LF::";
    std::vector<std::string> result;
    std::size_t pos = input.find("::");

    std::string beforeDoubleColon = input.substr(0, pos);
    result.push_back(beforeDoubleColon);

    std::string afterDoubleColon = input.substr(pos + 2);
    if (beforeDoubleColon.empty() || afterDoubleColon.empty())
    {
        cout << "invalid input" << endl;
    }

    std::istringstream afterStream(afterDoubleColon);
    std::string token;
    while (std::getline(afterStream, token, '.'))
    {
        result.push_back(token);
    }

    return 0;
}