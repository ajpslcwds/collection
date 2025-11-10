#include "nlohmann/json.hpp"
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

using json = nlohmann::json;

int main1(int argc, char *argv[])
{
    json jsonAll;
    for (int i = 0; i < 1; i++)
    {
        json jsonData;
        jsonData["key"] = i;
        jsonData["errorcode"] = 100 + i;

        jsonData["value"] = json::array();
        json &jsonArray = jsonData["value"];

        char sls1[100] = "hello world";
        sls1[8] = 0;
        std::string nValue = "";
        uint8_t valLen = 15;
        nValue.assign(sls1, valLen);
        jsonArray.push_back(sls1);
        jsonArray.push_back(nValue);

        int64_t ltime = 99999999999999;
        jsonArray.push_back(ltime);
        // jsonArray.push_back(to_string(ltime));

        jsonArray.push_back(i);
        unsigned char char1 = 0xff;
        jsonArray.push_back(char1);
        try
        {

            json jsonstr = std::string(1, char1);
            jsonstr.dump();
            jsonArray.push_back(std::string(1, char1));
        }
        catch (std::exception &e)
        {
            std::cout << "error: " << e.what() << std::endl;
        }
        catch (...)
        {
            std::cout << "unknown error" << std::endl;
        }
        jsonAll.push_back(jsonData);
    }
    string res = jsonAll.dump(2);
    std::cout << res << std::endl;
    return 0;
}

