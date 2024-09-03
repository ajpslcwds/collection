#include <iostream>
#include <boost/asio.hpp>
#include <thread>

using boost::asio::ip::tcp;

class Client {
public:
    Client(boost::asio::io_context& io_context, const tcp::resolver::results_type& endpoints)
        : socket_(io_context) {
        do_connect(endpoints);
    }

    void write(const std::string& message) {
        boost::asio::post(socket_.get_executor(),
            [this, message]() {
                do_write(message);
            });
    }

    void close() {
        boost::asio::post(socket_.get_executor(),
            [this]() { socket_.close(); });
    }

private:
    void do_connect(const tcp::resolver::results_type& endpoints) {
        boost::asio::async_connect(socket_, endpoints,
            [this](boost::system::error_code ec, tcp::endpoint) {
                if (!ec) {
                    std::cout << "Connected to the server." << std::endl;
                    do_read();
                }
            });
    }

    void do_read() {
        boost::asio::async_read_until(socket_,
            boost::asio::dynamic_buffer(input_buffer_), "\n",
            [this](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    std::string data(input_buffer_.substr(0, length - 1)); // 去除 '\n'
                    input_buffer_.erase(0, length);

                    std::cout << "Received from server: " << data << std::endl;

                    // 继续监听服务器发送的数据
                    do_read();
                } else {
                    socket_.close();
                }
            });
    }

    void do_write(const std::string& message) {
        bool write_in_progress = !output_buffer_.empty();
        output_buffer_ += message + "\n";
        if (!write_in_progress) {
            boost::asio::async_write(socket_,
                boost::asio::buffer(output_buffer_),
                [this](boost::system::error_code ec, std::size_t length) {
                    if (!ec) {
                        output_buffer_.erase(0, length);
                        if (!output_buffer_.empty()) {
                            do_write("");
                        }
                    } else {
                        socket_.close();
                    }
                });
        }
    }

    tcp::socket socket_;
    std::string input_buffer_;
    std::string output_buffer_;
};

int main() {
    try {
        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("127.0.0.1", "12345");
        Client c(io_context, endpoints);

        std::thread t([&io_context]() { io_context.run(); });

        while (true) {
            // 从标准输入读取消息并发送
            std::string message;
            std::cout << "Enter message: ";
            std::getline(std::cin, message);
            if (message == "exit") {
                c.close();
                break;
            }
            c.write(message);
        }

        t.join();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
