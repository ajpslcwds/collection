#include <cxxabi.h>

#include <iostream>
#include <typeinfo>

template <typename T>
std::string demangle(const T& value) {
  int status;
  char* realname = abi::__cxa_demangle(typeid(value).name(), 0, 0, &status);
  std::string result(realname);
  std::free(realname);
  return result;
}

struct A {
  int x;
  int y;
};

int main() {
  uint16_t a = 10;
  float b = 3.14;
  A aa;

  std::cout << demangle(a) << '\n';   // unsigned short
  std::cout << demangle(b) << '\n';   // float
  std::cout << demangle(aa) << '\n';  // A
}