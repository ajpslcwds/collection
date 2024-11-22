#include <iostream>

void printStrings(int count, char **strings)
{
    char *temp[] = strings;
    for (int i = 0; i < count; ++i)
    {
        std::cout << strings[i] << std::endl;
    }
}

int main()
{
    char strings[][256] = {"Hello", "World", "C++"}; // no

    // char *strings[] = {"Hello", "World", "C++"}; // yes
    printStrings(3, (char **)strings);
    return 0;
}
