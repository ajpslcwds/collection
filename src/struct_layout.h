#pragma once

#include "db_loader.h"
#include <atomic>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace data_attr
{
struct VarAttr
{
    uint32_t type_id = 0;
    TypeExtraPtr type_extra = nullptr;
    uint32_t offset = 0;
    uint32_t length = 0;

    VarAttr &operator=(const VarAttr &other)
    {
        if (this != &other)
        {
            type_id = other.type_id;
            type_extra = other.type_extra;
            offset = other.offset;
            length = other.length;
        }
        return *this;
    };
};
struct ModelPropLayout
{
    std::string prop_name = "";
    uint32_t offset = 0;
    uint32_t length = 0;
    uint32_t align = 0;
    uint32_t type_id = 0;
    TypeExtraPtr type_extra = nullptr; // save space, only for str,utd,array
};
using ModelPropLayoutPtr = std::shared_ptr<ModelPropLayout>;

struct ModelInfoLayout
{
    uint32_t parent_model_id = 0;
    uint32_t info_id = 0;
    std::string prop_name = ""; // 叶子成员全路径，例如 U1V1.ARR_U1V1[0].DINT1
    uint32_t type_id = 0;
    TypeExtraPtr type_extra = nullptr; // string:  strLength
    uint32_t offset = 0;
    uint32_t length = 0;
};
using ModelInfoLayoutPtr = std::shared_ptr<ModelInfoLayout>;
struct ModelLayout
{
    uint32_t model_id = 0;
    std::string model_name = "";
    uint32_t total_length = 0;
    uint32_t align = 4;
    std::vector<ModelPropLayoutPtr> props;
    std::unordered_map<std::string, ModelPropLayoutPtr> prop_name_map;

    std::vector<ModelInfoLayoutPtr> infos;
    std::unordered_map<std::string, ModelInfoLayoutPtr> info_name_map;
};

// ============================================================
// 变量属性布局结果（叶子成员）
// ============================================================
struct VarPropLayout
{
    uint32_t prop_id = 0;
    uint32_t tag_id = 0;
    std::string prop_name = ""; // 全路径，例如 STD::DS_DMODEL1.DINT1
    uint32_t type_id = 0;
    TypeExtraPtr type_extra = nullptr;
    uint32_t offset = 0;
    uint32_t length = 0;
};
using VarPropLayoutPtr = std::shared_ptr<VarPropLayout>;

// ============================================================
// 变量布局结果（按变量聚合）
// ============================================================
struct VarLayout
{
    uint32_t tag_id = 0;
    std::string tag_name = "";
    uint32_t type_id = 0;
    uint32_t total_length = 0;
    TypeExtraPtr type_extra = nullptr;
    std::vector<VarPropLayoutPtr> props;
    std::unordered_map<std::string, VarPropLayoutPtr> prop_name_map;
};
using VarLayoutPtr = std::shared_ptr<VarLayout>;

class StructLayoutCalculator
{
  public:
    ~StructLayoutCalculator() = default;

    int32_t Init(const DbConfig &dbConfig);
    VarPropLayoutPtr GetTagAttr(const std::string &tagName);

    static StructLayoutCalculator &GetInstance()
    {
        static StructLayoutCalculator instance;
        return instance;
    }

    const std::unordered_map<uint32_t, ModelLayout> &GetModelLayouts() const
    {
        return m_modelLayouts;
    }

    const std::unordered_map<uint32_t, VarLayoutPtr> &GetVarLayouts() const
    {
        return m_varLayouts;
    }

  private:
    StructLayoutCalculator() = default;
    StructLayoutCalculator(const StructLayoutCalculator &) = delete;
    StructLayoutCalculator &operator=(const StructLayoutCalculator &) = delete;
    int32_t LoadDbRecord();
    int32_t Calculate();

    uint32_t GetTypeAlign(uint32_t typeId, const std::string &strTypeExtra, const TypeExtra &typeExtra);
    uint32_t GetTypeLength(uint32_t typeId, const std::string &strTypeExtra, const TypeExtra &typeExtra,
                           uint32_t modelAlign);
    bool ResolvePathInModel(uint32_t modelId, const std::string &path, VarAttr &varAttr);
    int32_t CalculateModelLayout(const ModelRecordPtr &model, const std::vector<ModelPropRecordPtr> &props);
    int32_t CalculateModelInfoLayouts();
    int32_t CalculateVarLayouts();
    int32_t CalculateVarPropLayouts();
    int32_t CalculateVarPropLayoutDetail(VarLayout &var, const std::string &propName, uint32_t propId = 0);
    bool CalcuatePathInModel(uint32_t modelId, const std::string &path, VarAttr &varAttr);

  private:
    std::atomic<bool> m_isInit = {false};
    DbCache m_cache;
    std::unique_ptr<DbLoader> m_loader = nullptr;

    std::unordered_map<uint32_t, ModelLayout> m_modelLayouts;
    std::unordered_map<uint32_t, VarLayoutPtr> m_varLayouts;          // key: tag_id, var
    std::unordered_map<std::string, VarLayoutPtr> m_varLayoutsByName; // key: tag_id, var
    std::unordered_map<std::string, VarPropLayoutPtr> m_tagAttrMap;   // key: tagName, varProp  含域名且唯一
};
} // namespace data_attr