#include <iostream>

// 基础模板：递归计算 N!
template <unsigned int N>
struct Factorial {
    static constexpr unsigned int value = N * Factorial<N - 1>::value;
};

// 特化：终止条件，0! = 1
template <>
struct Factorial<0> {
    static constexpr unsigned int value = 1;
};

int main_digui() {
    const int a  = 10;
    std::cout << Factorial<a>::value << std::endl; // 输出 120 (5! = 5 * 4 * 3 * 2 * 1)
    return 0;
}

#include <type_traits>

// 仅当 T 是整数类型时启用此函数
template <typename T>
typename std::enable_if<std::is_integral<T>::value, void>::type
printType(T value) {
    std::cout << "Integral type: " << value << std::endl;
}

// 仅当 T 是浮点类型时启用此函数
template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, void>::type
printType(T value) {
    std::cout << "Floating point type: " << value << std::endl;
}

int main_enable() {
    printType(42);     // 输出 "Integral type: 42"
    printType(3.14);   // 输出 "Floating point type: 3.14"
    // printType("hello"); // 编译错误，无匹配函数
    return 0;
}

// 递归模板：展开循环
template <int N>
struct Unroll {
    static void print() {
        std::cout << N << " ";
        Unroll<N - 1>::print();
    }
};

// 特化：终止条件
template <>
struct Unroll<0> {
    static void print() {
        std::cout << 0 << std::endl;
    }
};

int main_zhankai() {
    Unroll<5>::print(); // 输出 5 4 3 2 1 0
    return 0;
}