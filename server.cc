#include <boost/asio.hpp>
#include <iostream>
#include <thread>

using boost::asio::ip::tcp;

void handle_client(tcp::socket& socket) {
  try {
    while (true) {
      // 接收数据
      char data[1024];
      boost::system::error_code error;
      size_t length = socket.read_some(boost::asio::buffer(data), error);

      if (error == boost::asio::error::eof) {
        // 连接关闭
        break;
      } else if (error) {
        throw boost::system::system_error(error);
      }

      std::cout << "Received: " << std::string(data, length) << std::endl;

      // 发送响应
      std::string response = "Message received";
      boost::asio::write(
          socket, boost::asio::buffer(response + std::string(data, length)),
          error);
    }
  } catch (std::exception& e) {
    std::cerr << "Exception in client handling: " << e.what() << std::endl;
  }
}

int main() {
  try {
    boost::asio::io_context io_context;

    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 12345));

    std::cout << "Server started, waiting for connections..." << std::endl;

    while (true) {
      tcp::socket socket(io_context);
      acceptor.accept(socket);
      std::cout << "Client connected!" << std::endl;
      handle_client(socket);
    }
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }

  return 0;
}
