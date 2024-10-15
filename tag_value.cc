#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>
using namespace std;

struct TagValue {
  size_t length = 0;
  char *buffer = nullptr;

  TagValue() = default;
  TagValue(const char *str, const int len) {
    Clear();
    length = len;
    buffer = new char[length];
    memcpy(buffer, str, length);
  }

  void Clear() {
    if (buffer != nullptr) {
      delete[] buffer;
      buffer = nullptr;
    }
    length = 0;
  }

  ~TagValue() {
    std::cout << "~TagValue:" << length << endl;
    Clear();
  }

  char *Data() const { return buffer; }
  size_t Length() const { return length; }

  void Display() const {
    cout << "length: " << length << ", data: " << (buffer ? buffer : "null")
         << endl;
  }
};

void GetTagValue(TagValue **tag_values, int *len) {
  static int num = 10;
  *len = (num += 3);
  *tag_values = new TagValue[*len];

  for (int i = 0; i < *len; i++) {
    (*tag_values)[i].length = 10 + i;
    (*tag_values)[i].buffer =
        new char[(*tag_values)[i].length + 1];  // +1用于存放字符串结尾的空字符
    strcpy((*tag_values)[i].buffer, "hello");
    strcat((*tag_values)[i].buffer, to_string(i).c_str());
  }
}

void freeTagValue(TagValue *tag_values) {
  if (tag_values) {
    delete[] tag_values;  // 释放tag_values数组本身
    tag_values = nullptr;
  }
}

int main() {
  for (int n = 0; n < 3; n++) {
    TagValue *tag_values = nullptr;
    int len = 0;
    GetTagValue(&tag_values, &len);

    for (int i = 0; i < len; i++) {
      strcat(tag_values[i].Data(), "--");
      tag_values[i].Display();
    }
    // 释放分配的内存
    freeTagValue(tag_values);
  }
  std::cout << "----------------" << std::endl;
  return 0;
}
