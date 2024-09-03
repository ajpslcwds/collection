#include <boost/asio.hpp>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

using boost::asio::ip::tcp;

class DRSdkSocket
{
  public:
    DRSdkSocket(boost::asio::io_context &io_context) : socket_(io_context), io_context_(io_context)
    {
    }

    // 作为客户端连接到服务器
    void connect(const std::string &host, const std::string &port)
    {
        tcp::resolver resolver(io_context_);
        auto endpoints = resolver.resolve(host, port);
        boost::asio::connect(socket_, endpoints);
    }

    // 作为服务器等待客户端连接
    void accept(tcp::acceptor &acceptor)
    {
        acceptor.accept(socket_);
    }

    // 同步发送数据
    void send(const std::string &message)
    {
        boost::asio::write(socket_, boost::asio::buffer(message));
    }

    // 同步接收数据
    std::string receive()
    {
        char data[1024];
        size_t length = socket_.read_some(boost::asio::buffer(data));
        return std::string(data, length);
    }

    // 异步接收数据
    void async_receive(const std::function<void(const std::string &)> &handler)
    {
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
                                [this, handler](boost::system::error_code ec, std::size_t length) {
                                    if (!ec)
                                    {
                                        handler(std::string(data_, length));
                                        // 继续监听接收数据
                                        async_receive(handler);
                                    }
                                });
    }

    // 关闭socket
    void close()
    {
        socket_.close();
    }

  private:
    tcp::socket socket_;
    boost::asio::io_context &io_context_;
    enum
    {
        max_length = 1024
    };
    char data_[max_length];
};

int main(int argc, char *argv[])
{

    bool cs_flag = argc > 1 ? true : false;

    try
    {

        if (cs_flag)
        {
            boost::asio::io_context io_context;
            // 示例用法: 作为服务器
            DRSdkSocket server_socket(io_context);
            tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 12344));
            server_socket.accept(acceptor);
            std::cout << "Client connected!" << std::endl;

            // 异步接收数据
            server_socket.async_receive(
                [](const std::string &message) { std::cout << "Received: " << message << std::endl; });

            // 服务器发送数据
            std::thread([&server_socket]() {
                while (true)
                {
                    std::string message;
                    std::cout << "Enter message to send: ";
                    std::getline(std::cin, message);
                    server_socket.send(message);
                }
            }).detach();
            io_context.run();
        }
        else
        {
            boost::asio::io_context io_context;

            // 客户端示例
            DRSdkSocket client_socket(io_context);
            client_socket.connect("127.0.0.1", "12344");
            std::cout << "Connected to server!" << std::endl;

            // 异步接收数据
            client_socket.async_receive(
                [](const std::string &message) { std::cout << "Received: " << message << std::endl; });

            // 客户端发送数据
            std::thread([&client_socket]() {
                while (true)
                {
                    std::string message;
                    std::cout << "Enter message to send: ";
                    std::getline(std::cin, message);
                    client_socket.send(message);
                }
            }).detach();
            io_context.run();
        }

        while (1)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
