#ifndef DRSDK_SOCKET_H
#define DRSDK_SOCKET_H

#include "drsdk_common.h"
#include <boost/asio.hpp>
#include <functional>
#include <iostream>
#include <mutex>
#include <set>
#include <thread>
#include <vector>

namespace drsdk
{

using boost::asio::ip::tcp;
constexpr uint16_t BUFFER_MAX_LENGTH = 1024;
class DRSdkSocket
{
  public:
    // Constructor
    DRSdkSocket(boost::asio::io_context &io_context);
    ~DRSdkSocket();

    // Synchronous methods with error handling

    // Synchronous methods with error handling
    void Connect(const std::string &host, const std::string &port, boost::system::error_code &ec);
    void Accept(tcp::acceptor &acceptor, boost::system::error_code &ec);

    // Asynchronous methods
    void AsyncAccept(tcp::acceptor &acceptor, const std::function<void(const boost::system::error_code &)> &handler);
    void AsyncConnect(const std::string &host, const std::string &port,
                      const std::function<void(const boost::system::error_code &)> &handler);

    // Synchronous send/receive with error handling
    void Write(const char *message, size_t length, boost::system::error_code &ec);
    std::string Read(boost::system::error_code &ec);

    // Asynchronous send/receive
    void AsyncWrite(const std::string &message,
                    const std::function<void(const boost::system::error_code &, std::size_t)> &handler);
    void AsyncRead(const std::function<void(const std::string &, const boost::system::error_code &)> &handler);

    // Close socket
    void Close();

    tcp::socket &Socket()
    {
        return socket_;
    }

  private:
    tcp::socket socket_;
    boost::asio::io_context &io_context_;
    char data_[BUFFER_MAX_LENGTH]; // Buffer for async read operations
    std::mutex write_mutex_;       // Mutex to protect send operations
};

} // namespace drsdk

#endif // DRSDK_SOCKET_H
