#ifndef DRSDK_H
#define DRSDK_H

#include <string>

#include <functional>
#include <vector>

#include "drsdk_manager.h"

// 符号导出宏定义
#ifdef _WIN32
#ifdef DRSDK_EXPORTS
#define DRSDK_API __declspec(dllexport)
#else
#define DRSDK_API __declspec(dllimport)
#endif
#else
#define DRSDK_API
#endif

namespace drsdk
{

DRSDK_API int32_t dr_get_id(const std::string &tag_name, uint32_t *value);
DRSDK_API int32_t dr_control_data(const std::string &tag_name, const std::string &value, const uint8_t write_mode);
DRSDK_API int32_t dr_read_data(const std::string &tag_name, std::string *value);
DRSDK_API int32_t dr_async_reg_tags(const std::vector<std::string> &tag_names, const Callback &call_back);
DRSDK_API int32_t dr_async_reg_tag(const std::string &tag_name, const Callback &call_back);

} // namespace drsdk

#endif // DRSDK_H
