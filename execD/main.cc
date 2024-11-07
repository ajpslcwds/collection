#include <dlfcn.h>
#include <iostream>

typedef void (*Func_C_Ptr)();

int main()
{
    // 动态加载 libC.so
    void *handle = dlopen("/home/wzq/code/wubuzhidao/build/libC/liblibC.so", RTLD_LAZY);
    if (!handle)
    {
        std::cerr << "Failed to open libC.so: " << dlerror() << std::endl;
        return 1;
    }

    // 重置 dlerror
    dlerror();

    // 获取 Func_C 的函数指针
    Func_C_Ptr Func_C = (Func_C_Ptr)dlsym(handle, "Func_C");
    const char *dlsym_error = dlerror();
    if (dlsym_error)
    {
        std::cerr << "Failed to locate Func_C: " << dlsym_error << std::endl;
        dlclose(handle);
        return 1;
    }

    // 调用 Func_C
    std::cout << "This is execD calling Func_C" << std::endl;
    Func_C();

    // 关闭库
    dlclose(handle);
    return 0;
}
