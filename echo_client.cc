#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;
enum
{
    BUF_SIZE = 1024
};
class Client
{
  public:
    Client(boost::asio::io_context &ioc, const std::string &host, const std::string &port)
        : socket_(ioc), resolver_(ioc)
    {
        resolver_.async_resolve(tcp::v4(), host, port,
                                std::bind(&Client::OnResolve, this, std::placeholders::_1, std::placeholders::_2));
    }

    void OnResolve(boost::system::error_code ec, tcp::resolver::results_type endpoints)
    {
        if (ec)
        {
            std::cerr << "Resolve: " << ec.message() << std::endl;
        }
        else
        {
            boost::asio::async_connect(
                socket_, endpoints, std::bind(&Client::OnConnect, this, std::placeholders::_1, std::placeholders::_2));
        }
    }

    void OnConnect(boost::system::error_code ec, tcp::endpoint endpoint)
    {
        if (ec)
        {
            std::cout << "Connect failed: " << ec.message() << std::endl;
            socket_.close();
        }
        else
        {
            DoWrite();
        }
    }

    void DoWrite()
    {
        std::size_t len = 0;
        do
        {
            std::cout << "Enter message: ";
            std::cin.getline(cin_buf_, BUF_SIZE);
            len = strlen(cin_buf_);
        } while (len == 0);

        boost::asio::async_write(socket_, boost::asio::buffer(cin_buf_, len),
                                 std::bind(&Client::OnWrite, this, std::placeholders::_1));
    }

    void OnWrite(boost::system::error_code ec)
    {
        if (!ec)
        {
            std::cout << "Reply is: ";

            socket_.async_read_some(boost::asio::buffer(buf_),
                                    std::bind(&Client::OnRead, this, std::placeholders::_1, std::placeholders::_2));
        }
    }

    void OnRead(boost::system::error_code ec, std::size_t length)
    {
        if (!ec)
        {
            std::cout.write(buf_.data(), length);
            std::cout << std::endl;
            // 如果想继续下一轮，可以在这里调用 DoWrite()。
            DoWrite();
        }
    }

  private:
    tcp::socket socket_;
    tcp::resolver resolver_;

    char cin_buf_[BUF_SIZE];
    std::array<char, BUF_SIZE> buf_;
};

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
        return 1;
    }

    const char *host = argv[1];
    const char *port = argv[2];

    boost::asio::io_context ioc;
    Client client(ioc, host, port);

    ioc.run();
    return 0;
}