/*  Filename:    drsdk_inner.cpp
 *  Copyright:   Shanghai Baosight Software Co., Ltd.
 *
 *  Description: Declare drsdk API. for unit test
 *
 *  @author:     wuzheqiang
 *  @version:    09/10/2024	wuzheqiang	Initial Version
 **************************************************************/
#include "drsdk_inner.h"
#include <iomanip>
#include <sstream>
#include <vector>

namespace drsdk
{

/**
 * @brief          save data to storage
 * @param [in]     vecTagName  tag names
 * @param [out]    vecValue   tag values
 * @return         0 if success
 * @version        2024/09/29	wuzheqiang	Initial Version
 */
DRSDK_API int32 dr_save_data(const std::vector<std::string> &vecTagName, const std::vector<std::string> &vecValue)
{
    return DRSdkManager::Instance()->DrSaveData(vecTagName, vecValue);
}
} // namespace drsdk
