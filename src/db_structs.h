#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace data_attr
{
struct Range
{
    int start = 0;
    int end = 0;
};
struct TypeExtra
{
    std::vector<Range> range; // only one-dimensional
    std::string eleType = ""; // ELE_TYPE
    uint32_t modelId = 0;     // MODEL_ID
    uint32_t strLength = 0;   // STR_LENGTH

    // for log display
    std::string ToString() const
    {
        std::stringstream ss;

        ss << "[";
        if (!eleType.empty())
        {
            ss << " eleType=" << eleType;
        }
        if (modelId != 0)
        {
            ss << " modelId=" << modelId;
        }
        if (strLength != 0)
        {
            ss << " strLength=" << strLength;
        }
        if (range.size() > 0)
        {
            ss << " range(" << range[0].start << "-" << range[0].end << ")";
        }
        ss << " ]";
        return ss.str();
    }
};
using TypeExtraPtr = std::shared_ptr<TypeExtra>;
inline std::string DisplayTypeExtraPtr(const TypeExtraPtr &ptr)
{
    if (ptr)
    {
        return ptr->ToString();
    }
    else
        return std::string("[ null ]");
};

// ============================================================
// T_DD_SM_DRIVER - 驱动信息表
// ============================================================
struct DriverRecord
{
    uint32_t driver_id;      // DRIVER_ID
    std::string driver_name; // DRIVER_NAME
    std::string driver_type; // DRIVER_TYPE ('item'|'block')
    std::string desp;        // DESP
};
using DriverRecordPtr = std::shared_ptr<DriverRecord>;

// ============================================================
// T_DD_SM_CHANNEL - 通道表
// ============================================================
struct ChannelRecord
{
    uint32_t channel_id;      // CHANNEL_ID
    std::string channel_name; // CHANNEL_NAME
    std::string channel_type; // CHANNEL_TYPE
    uint32_t driver_id;       // DRIVER_ID
    std::string conf;         // CONF (LONGTEXT)
    std::string desp;         // DESP
    int8_t status;            // STATUS (0-禁用, 1-启用)
};
using ChannelRecordPtr = std::shared_ptr<ChannelRecord>;

// ============================================================
// T_DD_SM_DEVICE - 设备表
// ============================================================
struct DeviceRecord
{
    uint32_t device_id;      // DEVICE_ID
    uint32_t driver_id;      // DRIVER_ID
    std::string device_name; // DEVICE_NAME
    std::string desp;        // DESP
    std::string conf;        // CONF (LONGTEXT, 驱动配置xml)
};
using DeviceRecordPtr = std::shared_ptr<DeviceRecord>;

// ============================================================
// T_DD_SM_TYPE - 数据类型表
// ============================================================
struct TypeRecord
{
    uint32_t type_id;      // TYPE_ID
    std::string type_name; // TYPE_NAME
    uint32_t length;       // LENGTH
};
using TypeRecordPtr = std::shared_ptr<TypeRecord>;

// ============================================================
// T_DD_SM_MDL_GRP - 模型分组结构
// ============================================================
struct ModelGroupRecord
{
    uint32_t node_id;       // NODE_ID
    std::string node_name;  // NODE_NAME
    uint32_t parent_id;     // PARENT_ID (0 表示无父节点)
    int16_t node_type;      // NODE_TYPE (0-目录, 1-表格)
    std::string name_space; // NAMESPACE
};
using ModelGroupRecordPtr = std::shared_ptr<ModelGroupRecord>;

// ============================================================
// T_DD_SM_MDL - 模型表总览
// ============================================================
struct ModelRecord
{
    uint32_t model_id;        // MODEL_ID
    uint32_t node_id;         // NODE_ID
    std::string model_name;   // MODEL_NAME
    std::string name_space;   // NAMESPACE
    std::string desp;         // DESP
    std::string aligh_length; // ALIGH_LENGTH ('1'|'2'|'4'|'8')
};
using ModelRecordPtr = std::shared_ptr<ModelRecord>;

// ============================================================
// T_DD_SM_MDL_PROP - 模型属性表
// ============================================================
struct ModelPropRecord
{
    uint32_t prop_id;                // PROP_ID
    uint32_t model_id;               // MODEL_ID
    std::string prop_name;           // PROP_NAME
    uint32_t type_id;                // TYPE_ID
    std::string type_extra;          // TYPE_EXTRA (JSON)
    std::vector<uint8_t> init_value; // INIT_VALUE (BLOB)
    std::string desp;                // DESP
    int16_t control_enable;          // CONTROL_ENABLE
};
using ModelPropRecordPtr = std::shared_ptr<ModelPropRecord>;

// ============================================================
// T_DD_SM_MDL_INFO - 模型散点表
// ============================================================
struct ModelInfoRecord
{
    uint32_t id;                     // ID
    uint32_t parent_model_id;        // PARENT_MODEL_ID
    std::string prop_name;           // PROP_NAME (属性全称)
    uint32_t type_id;                // TYPE_ID
    std::vector<uint8_t> init_value; // INIT_VALUE (BLOB)
    std::string alias;               // ALIAS
    std::string desp;                // DESP
    int16_t control_enable;          // CONTROL_ENABLE
};
using ModelInfoRecordPtr = std::shared_ptr<ModelInfoRecord>;

// ============================================================
// T_DD_SM_VAR_GRP - 变量分组结构
// ============================================================
struct VarGroupRecord
{
    uint32_t node_id;       // NODE_ID
    std::string node_name;  // NODE_NAME
    uint32_t parent_id;     // PARENT_ID (0 表示无父节点)
    int16_t node_type;      // NODE_TYPE (0-目录, 1-表格)
    std::string name_space; // NAMESPACE
};
using VarGroupRecordPtr = std::shared_ptr<VarGroupRecord>;

// ============================================================
// T_DD_SM_VAR - 变量表
// ============================================================
struct VarRecord
{
    uint32_t tag_id;        // TAG_ID
    std::string tag_name;   // TAG_NAME
    uint32_t node_id;       // NODE_ID
    std::string name_space; // NAMESPACE
    uint32_t type_id;       // TYPE_ID
    std::string type_extra; // TYPE_EXTRA (JSON)
    std::string io_addr;    // IO_ADDR
    int32_t scan_intv;      // SCAN_INTV
    uint32_t device_id;     // DEVICE_ID (0 表示无)
};
using VarRecordPtr = std::shared_ptr<VarRecord>;

// ============================================================
// T_DD_SM_VAR_PROP - 变量基础表
// ============================================================
struct VarPropRecord
{
    uint32_t prop_id;                    // PROP_ID
    uint32_t tag_id;                     // TAG_ID
    std::string prop_name;               // PROP_NAME
    uint32_t type_id;                    // TYPE_ID
    std::string io_addr;                 // IO_ADDR
    std::vector<uint8_t> init_value;     // INIT_VALUE (BLOB)
    std::string desp;                    // DESP
    int16_t control_enable;              // CONTROL_ENABLE
    int32_t scan_intv;                   // SCAN_INTV
    uint32_t device_id;                  // DEVICE_ID
    std::string byte_order;              // BYTE_ORDER
    int16_t evt_ctrl_enable;             // EVT_CTRL_ENABLE
    int32_t evt_ctrl_prior;              // EVT_CTRL_PRIOR
    int32_t override_enable;             // OVERRIDE_ENABLE
    std::vector<uint8_t> override_value; // OVERRIDE_VALUE (BLOB)
    std::string modify_time;             // MODIFY_TIME (DATETIME as string)
    int16_t egu_enable;                  // EGU_ENABLE
    double raw_high;                     // RAW_HIGH
    double raw_low;                      // RAW_LOW
    double egu_high;                     // EGU_HIGH
    double egu_low;                      // EGU_LOW
    double limit_high;                   // LIMIT_HIGH
    double limit_low;                    // LIMIT_LOW
    int16_t zp_enable;                   // ZP_ENABLE
    double zp_cent;                      // ZP_CENT
    double zp_thres;                     // ZP_THRES
    std::string egu_lbl;                 // EGU_LBL
    std::string egu_open;                // EGU_OPEN
    std::string egu_close;               // EGU_CLOSE
};
using VarPropRecordPtr = std::shared_ptr<VarPropRecord>;
} // namespace data_attr