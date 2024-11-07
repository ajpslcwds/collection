add_executable(execD main.cc)

# 链接 dl 库，用于动态加载
target_link_libraries(execD PRIVATE  dl )
