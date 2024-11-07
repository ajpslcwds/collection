#include <chrono>
#include <iostream>
#include <mutex>
#include <sstream>
#include <stdint.h>
#include <string.h>
#include <thread>

using namespace std;

int main()
{
    char temp[10] = "123456789";
    cout << temp << endl;
    strncpy(temp, "aaaaaaa", 3);
    cout << temp << endl;

    float f = 12345678.1234567;
    printf("%.7f\n", f);
        double d = 12345678.1234567;
    printf("%.7f\n", d);
    return 0;
}
