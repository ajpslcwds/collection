/*  Filename:    drsdk.cpp
 *  Copyright:   Shanghai Baosight Software Co., Ltd.
 *
 *  Description: Declare drsdk API
 *
 *  @author:     wuzheqiang
 *  @version:    09/10/2024	wuzheqiang	Initial Version
 **************************************************************/
#include "drsdk.h"
#include <iomanip>
#include <sstream>
#include <vector>

namespace drsdk
{

/**
 * @brief          init drsdk
 * @param [in]     strIp  dataservice ip
 * @param [in]     nPort  dataservice port
 * @param [in]     nTimeoutMs  socket timeout ms
 * @return         0 if success
 * @version        2024/09/25   wuzheqiang  Initial Version
 */
DRSDK_API int32 dr_sdk_init(const std::string &strIp, const int16 nPort, const uint16 nTimeoutMs)
{
    return DRSdkManager::Instance()->Init(strIp, nPort, nTimeoutMs);
}

/**
 * @brief          uninit drsdk
 * @return         0 if success
 * @version        2024/09/25	wuzheqiang	Initial Version
 */
DRSDK_API int32 dr_sdk_uninit()
{
    return DRSdkManager::Instance()->Uninit();
}

/**
 * @brief          get connection status
 * @return         true if connected else false
 * @version        2024/10/09	wuzheqiang	Initial Version
 */
DRSDK_API bool dr_connect_status()
{
    return DRSdkManager::Instance()->DrConnectStatus();
}

/**
 * @brief		Send control commands
 * @param [in]	vecTagName  tag names
 * @param [in]	vecValue   tag values
 * @return		0 if success
 * @version		09/10/2024	wuzheqiang	Initial Version
 */
DRSDK_API int32 dr_control_data(const std::vector<std::string> &vecTagName, const std::vector<std::string> &vecValue)
{
    return DRSdkManager::Instance()->DrControlData(vecTagName, vecValue);
}

/**
 * @brief		Synchronize data reading
 * @param [in]	vecTagName  tag names
 * @param [out]	vecValue   tag values
 * @return		0 if success
 * @version		09/10/2024	wuzheqiang	Initial Version
 */
DRSDK_API int32 dr_read_data(const std::vector<std::string> &vecTagName, std::vector<std::string> *vecValue)
{
    return DRSdkManager::Instance()->DrReadData(vecTagName, vecValue);
}

/**
 * @brief		register tag for async reading
 * @param [in]	vecTagName  tag names
 * @param [in]	nInterval   Interval for querying data, unit:ms
 * @param [in]	pCallBack   callback function ptr
 * @return		0 if success
 * @version		09/10/2024	wuzheqiang	Initial Version
 */
DRSDK_API int32 dr_async_reg_tag(const std::vector<std::string> &vecTagName, const uint16 nInterval,
                                 const Callback &pCallBack)
{
    return DRSdkManager::Instance()->DrAsyncRegTag(vecTagName, nInterval, pCallBack);
}

/**
 * @brief          get object attribute
 * @param [in]     objectName  object name
 * @param [out]    vecAttrs    object attributes
 * @return         0 if success
 * @version        2024/09/25	wuzheqiang	Initial Version
 */
DRSDK_API int32 dr_read_object_attr(const std::string &strObjectName, std::vector<drsdk::ObjectAttr> *vecAttrs)
{
    return DRSdkManager::Instance()->DrReadObjectAttr(strObjectName, vecAttrs);
}

/**
 * @brief          get object data
 * @param [in]     objectName  object name
 * @param [out]    vecAttrValues  object attribute  values
 * @return         0 if success
 * @version        2024/09/25	wuzheqiang	Initial Version
 */
DRSDK_API int32 dr_read_object_data(const std::string &strObjectName, std::vector<drsdk::ObjectData> *vecAttrValues)
{
    return DRSdkManager::Instance()->DrReadObectData(strObjectName, vecAttrValues);
}
} // namespace drsdk
