#include "drsdk_socket.h"
namespace drsdk
{

DRSdkSocket::DRSdkSocket(boost::asio::io_context &io_context) : socket_(io_context), io_context_(io_context)
{
}

DRSdkSocket::~DRSdkSocket()
{
    Close();
}
// --- Synchronous Methods ---

// Synchronous connect to a server with error handling
void DRSdkSocket::Connect(const std::string &host, const std::string &port, boost::system::error_code &ec)
{
    tcp::resolver resolver(io_context_);
    auto endpoints = resolver.resolve(host, port, ec);
    if (!ec)
    {
        boost::asio::connect(socket_, endpoints, ec);
    }
}

// Synchronous accept connection from a client with error handling
void DRSdkSocket::Accept(tcp::acceptor &acceptor, boost::system::error_code &ec)
{
    acceptor.accept(socket_, ec);
}

// Synchronous send data with error handling
void DRSdkSocket::Write(const char *message, size_t length, boost::system::error_code &ec)
{
    boost::asio::write(socket_, boost::asio::buffer(message, length), ec);
}

// Synchronous receive data with error handling
std::string DRSdkSocket::Read(boost::system::error_code &ec)
{
    char data[BUFFER_MAX_LENGTH];
    size_t length = socket_.read_some(boost::asio::buffer(data), ec);
    if (ec)
    {
        return "";
    }
    return std::string(data, length);
}

// --- Asynchronous Methods ---

// Asynchronous accept a new connection
void DRSdkSocket::AsyncAccept(tcp::acceptor &acceptor,
                              const std::function<void(const boost::system::error_code &)> &handler)
{
    acceptor.async_accept(socket_, [this, handler](boost::system::error_code ec) { handler(ec); });
}

// Asynchronous connect to a server
void DRSdkSocket::AsyncConnect(const std::string &host, const std::string &port,
                               const std::function<void(const boost::system::error_code &)> &handler)
{
    tcp::resolver resolver(io_context_);
    auto endpoints = resolver.resolve(host, port);

    boost::asio::async_connect(socket_, endpoints, [this, handler](boost::system::error_code ec, tcp::endpoint) {
        if (!ec)
        {
            LOG_INFO << "Connected to server!" << std::endl;
        }
        else
        {
            LOG_INFO << "Connection failed: " << ec.message() << std::endl;
        }
        handler(ec);
    });
}

// Asynchronous write data
void DRSdkSocket::AsyncWrite(const std::string &message,
                             const std::function<void(const boost::system::error_code &, std::size_t)> &handler)
{
    std::lock_guard<std::mutex> lock(write_mutex_); // Ensure thread-safety for write operations

    boost::asio::async_write(
        socket_, boost::asio::buffer(message),
        [this, handler](boost::system::error_code ec, std::size_t length) { handler(ec, length); });
}

// Asynchronous read data
void DRSdkSocket::AsyncRead(const std::function<void(const std::string &, const boost::system::error_code &)> &handler)
{
    socket_.async_read_some(boost::asio::buffer(data_, BUFFER_MAX_LENGTH),
                            [this, handler](boost::system::error_code ec, std::size_t length) {
                                if (!ec)
                                {
                                    handler(std::string(data_, length), ec);
                                }
                                else
                                {
                                    LOG_ERR << "Socket read error: " << ec.message() << std::endl;
                                    handler("", ec);
                                }
                            });
}

// Close the socket with error handling
void DRSdkSocket::Close()
{
    boost::system::error_code ec;
    socket_.close(ec);
    if (!ec)
    {
        LOG_ERR << "Socket close error: " << ec.message() << std::endl;
    }
}

} // namespace drsdk
