#include "drsdk_common.h"
#include "drsdk_socket.h"
#include <set>
using boost::asio::ip::tcp;
using drsdk::DataHeader;

class DsServer
{
  public:
    DsServer(boost::asio::io_context &io_context, short port)
        : io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
    {
        StartAccept();
    }

  private:
    void StartAccept()
    {
        // Create a new socket for the incoming connection
        auto new_socket = std::make_shared<drsdk::DRSdkSocket>(io_context_);

        // Asynchronously accept a new connection
        acceptor_.async_accept(new_socket->Socket(), [this, new_socket](boost::system::error_code ec) {
            if (!ec)
            {
                LOG_INFO << "Client connected" << std::endl;
                // Start reading from the new client asynchronously
                StartRead(new_socket);
                // Continue accepting other clients
                StartAccept();
            }
            else
            {
                LOG_ERR << "Accept error: " << ec.message() << std::endl;
            }
        });
    }

    void StartRead(std::shared_ptr<drsdk::DRSdkSocket> socket)
    {
        socket->AsyncRead([this, socket](const std::string &message, const boost::system::error_code &ec) {
            if (!ec)
            {

                LOG_INFO << "Received len: " << message.size() << std::endl;
                auto *data = message.c_str();
                DataHeader *head = (DataHeader *)data;
                head->output();
                switch (head->type)
                {
                case drsdk::SDK_READ_DATA:
                case drsdk::SDK_GET_ID: {
                    const char *tag_name = data + sizeof(DataHeader);
                    LOG_INFO << "tag_name: " << tag_name << std::endl;

                    // query the result

                    std::string value = "123123";
                    boost::system::error_code ec_inner;
                    socket->Write(value.c_str(), value.size(), ec_inner);
                    if (ec_inner)
                    {
                        LOG_ERR << "Write error: " << ec.message() << std::endl;
                    }
                    break;
                }
                case drsdk::SDK_CONTROL_DATA: {
                    const char *tag_name = data + sizeof(DataHeader);
                    LOG_INFO << "tag_name: " << tag_name << std::endl;

                    // send the value

                    std::string value = "1";
                    boost::system::error_code ec_inner;
                    socket->Write(value.c_str(), value.size(), ec_inner);
                    if (ec_inner)
                    {
                        LOG_ERR << "Write error: " << ec.message() << std::endl;
                    }
                    break;
                }
                case drsdk::SDK_REG_TAG: {
                    const char *tag_name = data + sizeof(DataHeader);
                    LOG_INFO << "tag_name: " << tag_name << std::endl;

                    reg_tag_names_.emplace(tag_name);

                    std::string value = "1";
                    boost::system::error_code ec_inner;
                    socket->Write(value.c_str(), value.size(), ec_inner);
                    if (ec_inner)
                    {
                        LOG_ERR << "Write error: " << ec.message() << std::endl;
                    }
                    break;
                }
                default:
                    LOG_INFO << "Not supported head->type: " << static_cast<u_int32_t>(head->type) << std::endl;
                    break;
                }

                // Continue reading from the same client
                StartRead(socket);
            }
            else
            {
                LOG_ERR << "Read error: " << ec.message() << std::endl;
            }
        });
    }

  private:
    boost::asio::io_context &io_context_;
    tcp::acceptor acceptor_;
    std::set<std::string> reg_tag_names_;
};

int main(int argc, char *argv[])
{

    try
    {
        boost::asio::io_context io_context;
        DsServer server(io_context, 12345);
        io_context.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
