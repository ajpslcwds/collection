/**
 * Filename        drsdk_transport_socket.h
 * Copyright       Shanghai Baosight Software Co., Ltd.
 * Description     Define DrSdkTransportSocket, as client use boost::asio to communicate with DataService.
 *
 * Author          wuzheqiang
 * Version         09/20/2024	wuzheqiang	Initial Version
 **************************************************************/

#ifndef DRSDK_TRANSPORT_SOCKET_H
#define DRSDK_TRANSPORT_SOCKET_H
#include "drsdk_common.h"
#include <atomic>
#include <boost/asio.hpp>
#include <mutex>
#include <queue>
#include <tuple>

namespace drsdk
{

class DrSdkTransportSocket
{
  public:
    using DataTuple = std::tuple<std::shared_ptr<char[]>, int32>; // buffer , length

    DrSdkTransportSocket();
    ~DrSdkTransportSocket();

    /**
     * @brief		init DrSdkTransportSocket: conenct to DataService
     * @param [in]     strIp  dataservice ip
     * @param [in]     nPort  dataservice port
     * @param [in]     nTimeoutMs  socket timeout ms
     * @return		0 if success
     * @version		09/10/2024	wuzheqiang	Initial Version
     */
    int32 Init(const std::string &strIp, const int16 nPort, const uint16 nTimeoutMs);

    /**
     * @brief		Close socket
     * @return		0 if success
     * @version		09/10/2024	wuzheqiang	Initial Version
     */
    int32 Close();

    /**
     * @brief		Send data to DataService
     * @param [in]	pSendBuf  buffer to send
     * @param [in]	nLenBuf   buffer length
     * @return		0 if success
     * @version		09/10/2024	wuzheqiang	Initial Version
     */
    int32 SendData(const char *pSendBuf, int32 nLenBuf);

    /**
     * @brief		Send data to DataService
     * @param [in]	pSendBuf  buffer to recv
     * @param [in]	nLenBuf   received length
     * @return		0 if success
     * @version		09/10/2024	wuzheqiang	Initial Version
     */
    int32 RecvData(char *pRecvBuf, int32 &pBufLen);

    /**
     * @brief          get connection status
     * @return         true if connected else false
     * @version        2024/10/09	wuzheqiang	Initial Version
     */
    bool GetConnectStatus() const
    {
        return m_bConnected;
    }

    /**
     * @brief          set connected callback, for automatic register async read callback
     * @version        2024/10/09	wuzheqiang	Initial Version
     */
    void SetConnectedCallback(const std::function<void()> &pConnectedCallback)
    {
        m_pConnectedCallback = pConnectedCallback;
    }

  private:
    /**
     * @brief          start connect to  dataseDataServicervice
     * @version        2024/09/23	wuzheqiang	Initial Version
     */
    void StartConnect();

    /**
     * @brief          after connected, start read data
     * @version        2024/09/23	wuzheqiang	Initial Version
     */
    void HandleConnect(const boost::system::error_code &error);

    /**
     * @brief          start read data
     * @version        2024/09/23	wuzheqiang	Initial Version
     */
    void StartRead();

    /**
     * @brief          read data head
     * @param [in]	   error  error code
     * @param [in]	   bytes_transferred   readed data length
     * @version        2024/09/23	wuzheqiang	Initial Version
     */
    void HandleReadHead(const boost::system::error_code &error, size_t bytes_transferred);

    /**
     * @brief          read data body
     * @param [in]	   error  error code
     * @param [in]	   bytes_transferred   readed data length
     * @version        2024/09/23	wuzheqiang	Initial Version
     */
    void HandleReadBody(const boost::system::error_code &error, size_t bytes_transferred);

    /**
     * @brief          Handle send data  result
     * @param [in]	   error  error code
     * @param [in]	   bytes_transferred   sent data length
     * @version        2024/09/23	wuzheqiang	Initial Version
     */
    void HandleSend(const boost::system::error_code &error, size_t bytes_transferred);

    /**
     * @brief          Start the reconnection process
     * @version        2024/09/23	wuzheqiang	Initial Version
     */
    void StartReconnect();

    /**
     * @brief          Handle reconnection result
     * @version        2024/09/23	wuzheqiang	Initial Version
     */
    void HandleReconnect(const boost::system::error_code &error);

    /**
     * @brief          put data to queue
     * @version        2024/09/23	wuzheqiang	Initial Version
     */
    bool Enqueue(DataTuple &tData);

    /**
     * @brief          get data to queue
     * @version        2024/09/23	wuzheqiang	Initial Version
     */
    bool Dequeue(DataTuple &tData);

  private:
    std::string m_strServerIp = "";
    uint16_t m_nServerPort = 0;
    boost::asio::io_context m_IoContext;
    std::unique_ptr<boost::asio::ip::tcp::socket> m_pSocket = nullptr;
    boost::asio::ip::tcp::resolver m_tResolver;
    std::atomic_bool m_bConnected = {false};
    boost::asio::steady_timer m_ReconnectTimer;           // Timer for delayed reconnection
    std::function<void()> m_pConnectedCallback = nullptr; // Callback when connected

    std::shared_ptr<char[]> m_RecvBufHead = nullptr;
    std::shared_ptr<char[]> m_RecvBufBody = nullptr;
    std::queue<DataTuple> m_SendQueue;
    std::mutex m_Mutex;
    std::condition_variable m_Cv;
    std::atomic_bool m_bStop = {false};
};
} // namespace drsdk
#endif