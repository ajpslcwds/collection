/**
 * Filename        drsdk_transport_shm.cpp
 * Copyright       Shanghai Baosight Software Co., Ltd.
 * Description     Define DrSdkTransportSocket, use share memory to communicate with DataService.
 *
 * Author          wuzheqiang
 * Version         09/20/2024	wuzheqiang	Initial Version
 **************************************************************/

#include "drsdk_transport_socket.h"
#include "error_code.h"
#include <boost/bind/bind.hpp>
namespace drsdk
{

// Constructor
DrSdkTransportSocket::DrSdkTransportSocket()
    : m_pSocket(std::make_unique<boost::asio::ip::tcp::socket>(m_IoContext)), m_tResolver(m_IoContext),
      m_ReconnectTimer(m_IoContext)
{
    m_bStop.store(false);
    m_bConnected.store(false);
    m_RecvBufHead.reset(new char[sizeof(DataHeader)]);
    m_RecvBufBody.reset(new char[RECV_BUFFER_MAX_LENGTH]);
}

// Destructor
DrSdkTransportSocket::~DrSdkTransportSocket()
{
    m_bStop.store(true);
    Close();
}
/**
 * @brief		init DrSdkTransportSocket: conenct to DataService
 * @param [in]     strIp  dataservice ip
 * @param [in]     nPort  dataservice port
 * @param [in]     nTimeoutMs  socket timeout ms
 * @return		0 if success
 * @version		09/10/2024	wuzheqiang	Initial Version
 */
int32 DrSdkTransportSocket::Init(const std::string &strIp, const int16 nPort, const uint16 nTimeoutMs)
{
    m_strServerIp = strIp;
    m_nServerPort = nPort;
    StartConnect(); // Start asynchronous connection

    // Start the io_context run loop, which will handle all async operations
    std::thread ioThread([this]() {
        m_IoContext.run(); // This will run in a separate thread
    });
    pthread_setname_np(ioThread.native_handle(), "io_context");
    ioThread.detach();

    // Timeout range: 500ms ~ 2000ms
    auto nRealTimeoutMs = (nTimeoutMs < 500) ? 500 : (nTimeoutMs > 2000) ? 2000 : nTimeoutMs;
    std::this_thread::sleep_for(std::chrono::milliseconds(nRealTimeoutMs)); // Wait for the connection to complete
    return m_bConnected ? EC_SDK_SUCCESS : EC_SDK_CONNECT_FAILED;
}

/**
 * @brief		Close socket
 * @return		0 if success
 * @version		09/10/2024	wuzheqiang	Initial Version
 */
int32 DrSdkTransportSocket::Close()
{
    if (m_bConnected)
    {
        boost::system::error_code ec;
        m_pSocket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        m_pSocket->close(ec);
        m_bConnected = false;
    }

    return EC_SDK_SUCCESS;
}

/**
 * @brief          start connect to  dataseDataServicervice
 * @version        2024/09/23	wuzheqiang	Initial Version
 */
void DrSdkTransportSocket::StartConnect()
{
    boost::asio::ip::tcp::resolver::results_type endpoints =
        m_tResolver.resolve(m_strServerIp, std::to_string(m_nServerPort));
    boost::asio::async_connect(
        *m_pSocket, endpoints,
        boost::bind(&DrSdkTransportSocket::HandleConnect, this, boost::asio::placeholders::error));
}

/**
 * @brief          after connected, start read data
 * @version        2024/09/23	wuzheqiang	Initial Version
 */
void DrSdkTransportSocket::HandleConnect(const boost::system::error_code &error)
{
    if (!error)
    {
        LOG_INFO << "Connected to server!" << std::endl;
        m_bConnected = true;
        StartRead(); // Start asynchronous read after successful connect

        if (nullptr != m_pConnectedCallback)
        {
            m_pConnectedCallback();
        }
    }
    else
    {
        LOG_ERR << "Failed to connect: " << error.message() << std::endl;
        StartReconnect(); // Start reconnection process if connection fails
    }
}

/**
 * @brief          start read data
 * @version        2024/09/23	wuzheqiang	Initial Version
 */
void DrSdkTransportSocket::StartRead()
{
    // m_pSocket->async_read_some(boost::asio::buffer(m_RecvBufHead),
    //                            boost::bind(&DrSdkTransportSocket::HandleRead, this, boost::asio::placeholders::error,
    //                                        boost::asio::placeholders::bytes_transferred));

    boost::asio::async_read(*m_pSocket, boost::asio::buffer(m_RecvBufHead.get(), sizeof(DataHeader)),
                            boost::asio::transfer_exactly(sizeof(DataHeader)), // Read exactly the size of DataHeader
                            boost::bind(&DrSdkTransportSocket::HandleReadHead, this, boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
}

/**
 * @brief          read data head
 * @param [in]	   error  error code
 * @param [in]	   bytes_transferred   readed data length
 * @version        2024/09/23	wuzheqiang	Initial Version
 */
void DrSdkTransportSocket::HandleReadHead(const boost::system::error_code &error, size_t bytes_transferred)
{
    if (!error)
    {
        LOG_INFO << "HandleReadHead Received data, bytes_transferred:" << bytes_transferred << std::endl;
        // Process the received data
        auto pHead = reinterpret_cast<DataHeader *>(m_RecvBufHead.get());
        LOG_INFO << "HandleReadHead Received data, head:" << pHead->ToString() << std::endl;
        if (pHead->nLen > 0)
        {

            boost::asio::async_read(*m_pSocket, boost::asio::buffer(m_RecvBufBody.get(), pHead->nLen),
                                    boost::asio::transfer_exactly(pHead->nLen), // Read exactly the size of msg body
                                    boost::bind(&DrSdkTransportSocket::HandleReadBody, this,
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));
        }
        else
        {
            int32 nMsgLen = sizeof(DataHeader);
            std::shared_ptr<char[]> pMsgBuf(new char[nMsgLen]);
            memcpy(pMsgBuf.get(), m_RecvBufHead.get(), sizeof(DataHeader));
            DataTuple tData = std::make_tuple(pMsgBuf, nMsgLen);
            Enqueue(tData);
            // Continue reading more data asynchronously
            StartRead();
        }
    }
    else
    {
        LOG_ERR << "Read error: " << error.message() << std::endl;
        StartReconnect(); // If read fails, trigger reconnection
    }
}

/**
 * @brief          read data body
 * @param [in]	   error  error code
 * @param [in]	   bytes_transferred   readed data length
 * @version        2024/09/23	wuzheqiang	Initial Version
 */
void DrSdkTransportSocket::HandleReadBody(const boost::system::error_code &error, size_t bytes_transferred)
{
    if (!error)
    {
        LOG_INFO << "HandleReadBody Received data, bytes_transferred:" << bytes_transferred << std::endl;

        int32 nMsgLen = sizeof(DataHeader) + bytes_transferred;
        std::shared_ptr<char[]> pMsgBuf(new char[nMsgLen]);
        memcpy(pMsgBuf.get(), m_RecvBufHead.get(), sizeof(DataHeader));
        memcpy(pMsgBuf.get() + sizeof(DataHeader), m_RecvBufBody.get(), bytes_transferred);
        DataTuple tData = std::make_tuple(pMsgBuf, nMsgLen);
        Enqueue(tData);
        // Continue reading more data asynchronously
        StartRead();
    }
    else
    {
        LOG_ERR << "Read error: " << error.message() << std::endl;
        StartReconnect(); // If read fails, trigger reconnection
    }
}

/**
 * @brief          put data to queue
 * @version        2024/09/23	wuzheqiang	Initial Version
 */
bool DrSdkTransportSocket::Enqueue(DataTuple &tData)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_SendQueue.emplace(std::move(tData));
    m_Cv.notify_one(); // Notify one waiting thread
    return true;
}

/**
 * @brief          get data to queue
 * @version        2024/09/23	wuzheqiang	Initial Version
 */
bool DrSdkTransportSocket::Dequeue(DataTuple &tData)
{
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_Cv.wait(lock, [this] { return m_bStop || !m_SendQueue.empty(); }); // Wait until the queue is not empty
    if (this->m_bStop)
    {
        return false; // If the stop flag is set£¬ return false
    }
    tData = m_SendQueue.front();
    m_SendQueue.pop();
    return true;
}

/**
 * @brief		Send data to DataService
 * @param [in]	pSendBuf  buffer to send
 * @param [in]	nLenBuf   buffer length
 * @return		0 if success
 * @version		09/10/2024	wuzheqiang	Initial Version
 */
int32 DrSdkTransportSocket::SendData(const char *pSendBuf, int32 nLenBuf)
{
    if (!m_bConnected)
    {
        LOG_ERR << "Not connected, can't send data" << std::endl;
        return EC_SDK_COMMON_ERROR;
    }

    boost::asio::async_write(*m_pSocket, boost::asio::buffer(pSendBuf, nLenBuf),
                             boost::bind(&DrSdkTransportSocket::HandleSend, this, boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));

    return EC_SDK_SUCCESS;
}

/**
 * @brief          send data hendler
 * @param [in]	   error  error code
 * @param [in]	   bytes_transferred   sent data length
 * @version        2024/09/23	wuzheqiang	Initial Version
 */
void DrSdkTransportSocket::HandleSend(const boost::system::error_code &error, size_t bytes_transferred)
{
    if (!error)
    {
        LOG_INFO << "Sent data successfully, bytes_transferred:" << bytes_transferred << std::endl;
    }
    else
    {
        LOG_ERR << "Send error: " << error.message() << std::endl;
        StartReconnect(); // If send fails, trigger reconnection
    }
}

/**
 * @brief		Send data to DataService
 * @param [in]	pSendBuf  buffer to recv
 * @param [in]	nLenBuf   received length
 * @return		0 if success
 * @version		09/10/2024	wuzheqiang	Initial Version
 */
int32 DrSdkTransportSocket::RecvData(char *pRecvBuf, int32 &pBufLen)
{
    DataTuple tData;
    if (!Dequeue(tData))
    {
        LOG_ERR << "RecvData Dequeue failed" << std::endl;
        return EC_SDK_COMMON_ERROR;
    }

    auto pRecvData = std::get<0>(tData);
    auto nRecvLen = std::get<1>(tData);
    std::memcpy(pRecvBuf, pRecvData.get(), nRecvLen);
    pBufLen = nRecvLen;

    return EC_SDK_SUCCESS;
}

/**
 * @brief          Start the reconnection process
 * @version        2024/09/23	wuzheqiang	Initial Version
 */
void DrSdkTransportSocket::StartReconnect()
{
    LOG_ERR << "Attempting to reconnect in 2 seconds..." << std::endl;

    // Wait for 5 seconds before attempting to reconnect
    m_ReconnectTimer.expires_after(std::chrono::seconds(2));
    m_ReconnectTimer.async_wait(
        boost::bind(&DrSdkTransportSocket::HandleReconnect, this, boost::asio::placeholders::error));
}
/**
 * @brief          Handle reconnection result
 * @version        2024/09/23	wuzheqiang	Initial Version
 */
void DrSdkTransportSocket::HandleReconnect(const boost::system::error_code &error)
{
    if (!error)
    {
        LOG_ERR << "Reconnecting..." << std::endl;
        Close();                                                                 // Close existing socket if necessary
        m_pSocket = std::make_unique<boost::asio::ip::tcp::socket>(m_IoContext); // Create a new socket
        StartConnect();                                                          // Try to reconnect
    }
}
} // namespace drsdk