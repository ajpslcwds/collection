

#include <iostream>
#include <sstream>
#include <string.h>
#include <string>
#include <vector>

using namespace std;

const std::string orin_data = "wzq.reg1@@ww@@1@";
const std::string split_char = "@@";

void SplitString(const std::string &strContent, const string &strDivideMark, vector<std::string> &vecRes)
{
    size_t nStart = 0;
    size_t nEnd = 0;
    while (nStart < strContent.size() && nEnd != string::npos)
    {
        nEnd = strContent.find(strDivideMark, nStart);

        if (nEnd != nStart)
        {
            vecRes.push_back(strContent.substr(nStart, nEnd - nStart));
        }
        nStart = nEnd + strDivideMark.size();
    }
}

int main()
{
    vector<string> vecRes;
    SplitString(orin_data, split_char, vecRes);
    cout << vecRes.size() << endl;
    for (auto &str : vecRes)
    {
        cout << str << endl;
    }
}