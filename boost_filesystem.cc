#include <boost/filesystem.hpp>
#include <iostream>

namespace fs = boost::filesystem;

int main(int argc, char *argv[])
{
    const char *dir_path_name = "/home/wzq/code/collection/";
    fs::path dir_path(dir_path_name);

    if (!fs::exists(dir_path) || !fs::is_directory(dir_path))
    {
        std::cerr << "Invalid directory path.\n";
        return 1;
    }

    try
    {
        for (fs::recursive_directory_iterator it(dir_path), end; it != end; ++it)
        {
            if (fs::is_regular_file(*it))
            {
                std::cout << it->path().string() << std::endl;
            }
        }
    }
    catch (const fs::filesystem_error &e)
    {
        std::cerr << "Filesystem error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
