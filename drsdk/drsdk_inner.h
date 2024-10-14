/*  Filename:    drsdk_inner.h
 *  Copyright:   Shanghai Baosight Software Co., Ltd.
 *
 *  Description: Declare drsdk inner API. for unit test
 *
 *  @author:     wuzheqiang
 *  @version:    10/08/2024	wuzheqiang	Initial Version
 **************************************************************/

#ifndef DRSDK_INNER_H
#define DRSDK_INNER_H

#include <string>

#include <functional>
#include <map>
#include <vector>

#include "drsdk_manager.h"

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
/**
 * @brief          save data to storage
 * @param [in]     vecTagName  tag names
 * @param [out]    vecValue   tag values
 * @return         0 if success
 * @version        2024/09/29	wuzheqiang	Initial Version
 */
DRSDK_API int32 dr_save_data(const std::vector<std::string> &vecTagName, const std::vector<std::string> &vecValue);

} // namespace drsdk

#endif // DRSDK_INNER_H
