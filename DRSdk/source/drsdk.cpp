#include "drsdk.h"
#include <iomanip>
#include <sstream>
#include <vector>

namespace drsdk
{
DRSDK_API int32_t dr_get_id(const std::string &tag_name, uint32_t *value)
{
    return DRSdkManager::Instance()->DrGetId(tag_name, value);
}

DRSDK_API int32_t dr_control_data(const std::string &tag_name, const std::string &value, const uint8_t write_mode)
{
    return DRSdkManager::Instance()->DrControlData(tag_name, value, write_mode);
}

DRSDK_API int32_t dr_read_data(const std::string &tag_name, std::string *value)
{
    return DRSdkManager::Instance()->DrReadData(tag_name, value);
}
DRSDK_API int32_t dr_async_reg_tag(const std::string &tag_name, const Callback &call_back)
{
    return DRSdkManager::Instance()->DrAsyncRegTag(tag_name, call_back);
}
DRSDK_API int32_t dr_async_reg_tags(const std::vector<std::string> &tag_names, const Callback &call_back)
{
    int res = 0;
    return res;
}

} // namespace drsdk
