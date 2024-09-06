#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;
enum
{
    BUF_SIZE = 1024
};
class Session : public std::enable_shared_from_this<Session>
{
  public:
    Session(tcp::socket socket) : socket_(std::move(socket))
    {
    }

    void Start()
    {
        DoRead();
    }

    void DoRead()
    {
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(buffer_),
                                [this, self](boost::system::error_code ec, std::size_t length) {
                                    if (!ec)
                                    {
                                        DoWrite(length);
                                    }
                                });
    }

    void DoWrite(std::size_t length)
    {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(buffer_, length),
                                 [this, self](boost::system::error_code ec, std::size_t length) {
                                     if (!ec)
                                     {
                                         DoRead();
                                     }
                                 });
    }

  private:
    tcp::socket socket_;
    std::array<char, BUF_SIZE> buffer_;
};

class Server
{
  public:
    Server(boost::asio::io_context &ioc, std::uint16_t port) : acceptor_(ioc, tcp::endpoint(tcp::v4(), port))
    {
        DoAccept();
    }

  private:
    void DoAccept()
    {
        acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec)
            {
                std::cout << "Accepted connection from " << socket.remote_endpoint() << std::endl;
                std::make_shared<Session>(std::move(socket))->Start();
            }
            DoAccept();
        });
    }

  private:
    tcp::acceptor acceptor_;
};
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    std::uint16_t port = std::atoi(argv[1]);

    boost::asio::io_context ioc;
    Server server(ioc, port);

    ioc.run();
    return 0;
}