#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main() {
    try {
        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve("127.0.0.1", "12345");

        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        std::cout << "Connected to the server." << std::endl;

        while (true) {
            // 发送数据
            std::string message;
            std::cout << "Enter message: ";
            std::getline(std::cin, message);

            boost::system::error_code error;
            boost::asio::write(socket, boost::asio::buffer(message), error);

            if (error) {
                throw boost::system::system_error(error);
            }

            // 接收响应
            char reply[1024];
            size_t reply_length = socket.read_some(boost::asio::buffer(reply), error);

            if (error == boost::asio::error::eof) {
                // 连接关闭
                break;
            } else if (error) {
                throw boost::system::system_error(error);
            }

            std::cout << "Reply from server: " << std::string(reply, reply_length) << std::endl;
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
