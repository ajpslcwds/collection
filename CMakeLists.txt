cmake_minimum_required(VERSION 3.14)
project(design_patterns LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 统一编译选项
add_compile_options(-Wall -Wextra -Wpedantic)

# 查找线程库（部分模式使用 std::thread / std::shared_mutex）
find_package(Threads REQUIRED)

# 定义一个函数简化添加模式示例
function(add_pattern_example category name)
    set(target_name "${category}_${name}")
    add_executable(${target_name} src/${category}/${name}/main.cpp)
    set_target_properties(${target_name} PROPERTIES
        OUTPUT_NAME ${name}
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${category}
    )
    target_link_libraries(${target_name} PRIVATE Threads::Threads)
endfunction()

# 创建型模式
add_pattern_example(creational singleton)
add_pattern_example(creational factory-method)
add_pattern_example(creational abstract-factory)
add_pattern_example(creational builder)
add_pattern_example(creational prototype)

# 结构型模式
add_pattern_example(structural adapter)
add_pattern_example(structural bridge)
add_pattern_example(structural composite)
add_pattern_example(structural decorator)
add_pattern_example(structural facade)
add_pattern_example(structural flyweight)
add_pattern_example(structural proxy)

# 行为型模式
add_pattern_example(behavioral chain-of-responsibility)
add_pattern_example(behavioral command)
add_pattern_example(behavioral iterator)
add_pattern_example(behavioral mediator)
add_pattern_example(behavioral memento)
add_pattern_example(behavioral observer)
add_pattern_example(behavioral state)
add_pattern_example(behavioral strategy)
add_pattern_example(behavioral template-method)
add_pattern_example(behavioral visitor)
add_pattern_example(behavioral interpreter)
