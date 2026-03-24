#include "struct_layout.h"
#include "db_loader.h"
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <cstdio>

#include <sstream>
#include <string>
#include <unordered_map>
using json = nlohmann::json;

namespace data_attr
{
struct PathToken
{
    std::string name = "";
    bool hasIndex = false;
    int index = 0;
};
void from_json(const json &j, Range &r)
{
    j.at("start").get_to(r.start);
    j.at("end").get_to(r.end);
}

void from_json(const json &j, TypeExtra &t)
{
    if (j.contains("RANGE"))
        j.at("RANGE").get_to(t.range);

    if (j.contains("ELE_TYPE"))
        j.at("ELE_TYPE").get_to(t.eleType);

    if (j.contains("MODEL_ID"))
        j.at("MODEL_ID").get_to(t.modelId);

    if (j.contains("STR_LENGTH"))
        j.at("STR_LENGTH").get_to(t.strLength);
}

static uint32_t ParseAlign(const std::string &alignStr)
{
    if (alignStr == "1")
        return 1;
    if (alignStr == "2")
        return 2;
    if (alignStr == "4")
        return 4;
    if (alignStr == "8")
        return 8;
    return 4;
}

static uint32_t StringToUint32(const std::string &s)
{
    try
    {
        return static_cast<uint32_t>(std::stoul(s));
    }
    catch (...)
    {
        return 0;
    }
}

static void TrimString(std::string &s)
{
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t' || s.front() == '"' || s.front() == '\''))
        s.erase(s.begin());
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '"' || s.back() == '\''))
        s.pop_back();
}

static std::string ToLower(const std::string &s)
{
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

static bool ParseTypeExtra(const std::string &str, TypeExtra &out)
{
    if (str.empty())
        return true;

    try
    {
        json j = json::parse(str);
        out = j.get<TypeExtra>();
        return true;
    }
    catch (const std::exception &e)
    {
        fprintf(stderr, "ParseTypeExtra error: %s\n", e.what());
        return false;
    }
}
static const ModelPropRecord *FindModelPropByName(
    const std::unordered_map<uint32_t, std::vector<ModelPropRecordPtr>> &props, uint32_t modelId,
    const std::string &propName)
{
    auto it = props.find(modelId);
    if (it == props.end())
        return nullptr;
    for (const auto &p : it->second)
    {
        if (p && p->prop_name == propName)
            return p.get();
    }
    return nullptr;
}

static const ModelPropLayoutPtr FindPropLayoutByName(const std::unordered_map<uint32_t, ModelLayout> &layouts,
                                                     uint32_t modelId, const std::string &propName)
{
    auto it = layouts.find(modelId);
    if (it == layouts.end())
    {
        return nullptr;
    }
    auto itName = it->second.prop_name_map.find(propName);
    if (itName != it->second.prop_name_map.end() && itName->second)
    {
        return itName->second;
    }
    return nullptr;
}

static const ModelInfoLayoutPtr FindModelInfoLayoutByName(const std::unordered_map<uint32_t, ModelLayout> &layouts,
                                                          uint32_t modelId, const std::string &infoName)
{
    auto it = layouts.find(modelId);
    if (it == layouts.end())
    {
        return nullptr;
    }
    auto itName = it->second.info_name_map.find(infoName);
    if (itName != it->second.info_name_map.end() && itName->second)
    {
        return itName->second;
    }
    return nullptr;
}

static bool GetArrayRange0(const TypeExtra &typeExtra, Range &outRange)
{
    if (typeExtra.range.empty())
        return false;
    outRange = typeExtra.range[0];
    if (outRange.end < outRange.start)
        return false;
    return true;
}

// 获取数组元素的长度
static bool GetArrayElementSize(const std::unordered_map<std::string, TypeRecordPtr> &types_by_name,
                                const std::unordered_map<uint32_t, ModelLayout> &layouts, const TypeExtra &typeExtra,
                                uint32_t &outEleSize)
{
    outEleSize = 0;

    // 数组元素是模型
    if (typeExtra.modelId > 0)
    {
        auto it = layouts.find(static_cast<uint32_t>(typeExtra.modelId));
        if (it == layouts.end())
            return false;
        outEleSize = it->second.total_length;
        return true;
    }

    // 数组元素是字符串
    if (typeExtra.eleType == "STRING")
    {
        outEleSize = static_cast<uint32_t>(typeExtra.strLength + 2);
        return true;
    }

    // 数组元素是基础类型

    auto it = types_by_name.find(typeExtra.eleType);
    if (it == types_by_name.end() || !it->second)
        return false;
    outEleSize = it->second->length;
    return true;
}

// 获取数组元素的类型（基本类型或者udt）
static bool GetArrayElementType(const std::unordered_map<std::string, TypeRecordPtr> &types_by_name,
                                const std::unordered_map<uint32_t, ModelLayout> &layouts, const TypeExtra &typeExtra,
                                uint32_t &outTypeId, TypeExtra &outTypeExtra)
{
    outTypeId = 0;

    // 数组元素是模型
    if (typeExtra.modelId > 0)
    {
        auto it = layouts.find(static_cast<uint32_t>(typeExtra.modelId));
        if (it == layouts.end())
            return false;
        outTypeId = 27;
        outTypeExtra.modelId = typeExtra.modelId;
        return true;
    }

    // 数组元素是基础类型
    auto it = types_by_name.find(typeExtra.eleType);
    if (it == types_by_name.end() || !it->second)
        return false;
    outTypeId = it->second->type_id;
    if (outTypeId == 17)
    {
        outTypeExtra.strLength = typeExtra.strLength;
    }
    return true;
}

static bool ParsePropPath(const std::string &path, std::vector<PathToken> &outTokens)
{
    outTokens.clear();
    if (path.empty())
        return false;

    size_t pos = 0;
    while (pos < path.size())
    {
        size_t dot = path.find('.', pos);
        std::string part = (dot == std::string::npos) ? path.substr(pos) : path.substr(pos, dot - pos);
        pos = (dot == std::string::npos) ? path.size() : dot + 1;

        TrimString(part);
        if (part.empty())
            return false;

        PathToken tok;
        size_t lb = part.find('[');
        if (lb == std::string::npos)
        {
            tok.name = part;
            outTokens.push_back(std::move(tok));
            continue;
        }

        size_t rb = part.find(']', lb + 1);
        if (rb == std::string::npos)
            return false;

        tok.name = part.substr(0, lb);
        TrimString(tok.name);
        if (tok.name.empty())
            return false;

        std::string idxStr = part.substr(lb + 1, rb - lb - 1);
        TrimString(idxStr);
        if (idxStr.empty())
            return false;

        try
        {
            tok.index = std::stoi(idxStr);
        }
        catch (...)
        {
            return false;
        }

        // 暂时只支持单维数组：name[3]，不支持 name[1][2]
        if (part.find('[', rb + 1) != std::string::npos)
            return false;

        // ] 后面不允许额外字符
        std::string tail = part.substr(rb + 1);
        TrimString(tail);
        if (!tail.empty())
            return false;

        tok.hasIndex = true;
        outTokens.push_back(std::move(tok));
    }

    return !outTokens.empty();
}

int32_t StructLayoutCalculator::Init(const DbConfig &dbConfig)
{

    m_loader = std::make_unique<DbLoader>(dbConfig);
    int32_t iRet = LoadDbRecord();
    if (iRet != 0)
    {
        printf("[ERROR] LoadDbRecord failed\n");
        return iRet;
    }

    printf("\n[INFO] Calculating model layouts...\n");
    iRet = Calculate();
    if (iRet != 0)
    {
        printf("[ERROR] Failed to calculate model layouts\n");
        return iRet;
    }
    return 0;
}
int32_t StructLayoutCalculator::LoadDbRecord()
{
    int32_t iRet = m_loader->Connect();
    if (iRet != 0)
    {
        printf("m_loader->Connect failed\n");
        return iRet;
    }

    // 加载全部表
    iRet = m_loader->LoadAll(m_cache);
    if (iRet != 0)
    {
        printf("m_loader->Connect failed\n");
        return iRet;
    }
    m_loader->Disconnect();

    // ----------------------------------------------------------
    // 简单统计输出
    // ----------------------------------------------------------
    printf("[INFO] Loaded records:\n"
           "  T_DD_SM_DRIVER    : %zu\n"
           "  T_DD_SM_CHANNEL   : %zu\n"
           "  T_DD_SM_DEVICE    : %zu\n"
           "  T_DD_SM_TYPE      : %zu\n"
           "  T_DD_SM_MDL_GRP   : %zu\n"
           "  T_DD_SM_MDL       : %zu\n"
           "  T_DD_SM_MDL_PROP  : %zu\n"
           "  T_DD_SM_MDL_INFO  : %zu\n"
           "  T_DD_SM_VAR_GRP   : %zu\n"
           "  T_DD_SM_VAR       : %zu\n"
           "  T_DD_SM_VAR_PROP  : %zu\n",
           m_cache.drivers.size(), m_cache.channels.size(), m_cache.devices.size(), m_cache.types.size(),
           m_cache.model_groups.size(), m_cache.models.size(), m_cache.model_props.size(), m_cache.model_infos.size(),
           m_cache.var_groups.size(), m_cache.vars.size(), m_cache.var_props.size());
    return 0;
}

uint32_t StructLayoutCalculator::GetTypeAlign(uint32_t typeId, const std::string &strTypeExtra,
                                              const TypeExtra &typeExtra)
{
    // basic type(except string)
    if (strTypeExtra.empty())
    {
        auto it = m_cache.types.find(typeId);
        if (it != m_cache.types.end() && it->second)
        {
            return it->second->length;
        }
        fprintf(stderr, "GetTypeAlign error: typeId=%u\n", typeId);
        return 4; // wuzheqiang
    }

    if (17 == typeId)
    {
        return 1;
    }

    const std::string &eleType = typeExtra.eleType;
    // UDT or array of UDT
    if (0 != typeExtra.modelId)
    {
        auto modelIt = m_cache.models.find(typeExtra.modelId);
        if (modelIt != m_cache.models.end() && modelIt->second)
        {
            return ParseAlign(modelIt->second->aligh_length);
        }
        fprintf(stderr, "GetTypeAlign error: typeExtra.modelId=%u\n", typeExtra.modelId);
        return 4; // wuzheqiang
    }

    // array of string
    if ("STRING" == eleType)
    {
        return 1;
    }

    // array of basic type(except string)
    auto typeIt = m_cache.types_by_name.find(eleType);
    if (typeIt != m_cache.types_by_name.end() && typeIt->second)
    {
        return typeIt->second->length;
    }

    fprintf(stderr, "GetTypeAlign error: typeExtra.eleType=%s\n", eleType.c_str());
    return 4; // wuzheqiang
}

uint32_t StructLayoutCalculator::GetTypeLength(uint32_t typeId, const std::string &strTypeExtra,
                                               const TypeExtra &typeExtra, uint32_t modelAlign)
{
    if (strTypeExtra.empty())
    {
        auto it = m_cache.types.find(typeId);
        if (it != m_cache.types.end() && it->second)
        {
            return it->second->length;
        }
        fprintf(stderr, "GetTypeLength error: typeId=%u\n", typeId);
        return 0; // wuzheqiang
    }

    if (17 == typeId)
    {
        return typeExtra.strLength + 2;
    }

    const std::string &eleType = typeExtra.eleType;
    const uint32_t modelId = typeExtra.modelId;
    if (modelId > 0)
    { // checkek modelId is valid
        auto modelIt = m_cache.models.find(modelId);
        if (modelIt == m_cache.models.end() || nullptr == modelIt->second)
        {
            fprintf(stderr, "GetTypeLength error:models modelId=%u\n", modelId);
            return 0; // wuzheqiang
        }
    }
    // udt: Do not have eletype
    if (eleType.empty() && modelId > 0)
    {
        auto resIt = m_modelLayouts.find(modelId);
        if (resIt == m_modelLayouts.end())
        {
            auto propsIt = m_cache.model_props.find(modelId);
            if (propsIt != m_cache.model_props.end())
            {
                CalculateModelLayout(m_cache.models.at(modelId), propsIt->second);
            }
        }
        resIt = m_modelLayouts.find(modelId);
        if (resIt != m_modelLayouts.end())
        {
            return resIt->second.total_length;
        }
        else
        {
            fprintf(stderr, "GetTypeLength error: modelProp modelId=%u\n", modelId);
            return 0; // wuzheqiang
        }
    }

    // array：have eletype
    assert(typeExtra.range.size() >= 1);
    const auto &range_0 = typeExtra.range[0];
    uint32_t arrayCount = range_0.end - range_0.start + 1;

    // array of udt
    if (modelId > 0)
    {
        auto resIt = m_modelLayouts.find(modelId);
        if (resIt == m_modelLayouts.end())
        {
            auto propsIt = m_cache.model_props.find(modelId);
            if (propsIt != m_cache.model_props.end())
            {
                CalculateModelLayout(m_cache.models.at(modelId), propsIt->second);
            }
        }
        resIt = m_modelLayouts.find(modelId);
        if (resIt != m_modelLayouts.end())
        {
            return resIt->second.total_length * arrayCount;
        }
        else
        {
            fprintf(stderr, "GetTypeLength error: modelProp modelId=%u\n", modelId);
            return 0; // wuzheqiang
        }
    }

    // array of string
    if ("STRING" == eleType)
    {
        return arrayCount * (typeExtra.strLength + 2);
    }
    // array of basic type(except string)
    auto typeIt = m_cache.types_by_name.find(eleType);
    if (typeIt != m_cache.types_by_name.end() && typeIt->second)
    {
        return arrayCount * typeIt->second->length;
    }

    fprintf(stderr, "GetTypeLength error: typeExtra.eleType=%s\n", eleType.c_str());
    return 0; // wuzheqiang
}

// 解析模型下的属性路径
// 可以是叶子节点，也可以是中间节点
bool StructLayoutCalculator::ResolvePathInModel(uint32_t modelId, const std::string &path, VarAttr &varAttr)
{
    std::vector<PathToken> tokens;
    if (!ParsePropPath(path, tokens))
    {
        fprintf(stderr, "ParsePropPath failed: modelId=%u path=%s\n", modelId, path.c_str());
        return false;
    }

    uint32_t curModelId = modelId;
    uint32_t baseOffset = 0;

    for (size_t i = 0; i < tokens.size(); i++)
    {
        const bool isLast = (i + 1 == tokens.size());
        const PathToken &tok = tokens[i];

        const ModelPropLayoutPtr propLayoutPtr = FindPropLayoutByName(m_modelLayouts, curModelId, tok.name);
        if (nullptr == propLayoutPtr)
        {
            fprintf(stderr, "member not found: modelId=%u member=%s path=%s\n", curModelId, tok.name.c_str(),
                    path.c_str());
            return false;
        }
        TypeExtraPtr typeExtraPtr = propLayoutPtr->type_extra;
        uint32_t curOffset = baseOffset + propLayoutPtr->offset;

        // 数组成员: name[idx]
        if (tok.hasIndex)
        {
            if (!typeExtraPtr || typeExtraPtr->range.empty())
            {
                fprintf(stderr, "not an array: modelId=%u member=%s\n", curModelId, tok.name.c_str());
                return false;
            }
            Range &r0 = typeExtraPtr->range[0];
            if (tok.index < r0.start || tok.index > r0.end)
            {
                fprintf(stderr, "index out of range : path = %s index = %d\n", path.c_str(), tok.index);
                return false;
            }

            uint32_t eleSize = 0;
            if (!GetArrayElementSize(m_cache.types_by_name, m_modelLayouts, *typeExtraPtr, eleSize) || eleSize == 0)
            {
                fprintf(stderr, "element size invalid: path=%s\n", path.c_str());
                return false;
            }

            curOffset += static_cast<uint32_t>(tok.index - r0.start) * eleSize;

            if (isLast)
            {
                uint32_t eleTypeId = 0;
                TypeExtra eleTypeExtra;
                if (!GetArrayElementType(m_cache.types_by_name, m_modelLayouts, *typeExtraPtr, eleTypeId, eleTypeExtra))
                {
                    fprintf(stderr, "element type invalid: path=%s\n", path.c_str());
                    return false;
                }
                varAttr.type_id = eleTypeId;
                if (eleTypeExtra.modelId > 0 || eleTypeExtra.strLength > 0)
                {
                    varAttr.type_extra = std::make_shared<TypeExtra>(eleTypeExtra);
                }
                varAttr.offset = curOffset;
                varAttr.length = eleSize;
                return true;
            }

            // 只有数组元素为模型，才允许继续往下解析
            if (typeExtraPtr->modelId <= 0)
            {
                fprintf(stderr, "path continues after non-UDT array: path=%s\n", path.c_str());
                return false;
            }

            curModelId = static_cast<uint32_t>(typeExtraPtr->modelId);
            baseOffset = curOffset;
            continue;
        }

        // 普通成员: name
        if (isLast)
        {
            varAttr.type_id = propLayoutPtr->type_id;
            varAttr.type_extra = propLayoutPtr->type_extra;
            varAttr.offset = curOffset;
            varAttr.length = propLayoutPtr->length;
            return true;
        }

        if (!typeExtraPtr || typeExtraPtr->modelId <= 0)
        {
            fprintf(stderr, "intermediate must be UDT: path=%s member=%s\n", path.c_str(), tok.name.c_str());
            return false;
        }

        curModelId = static_cast<uint32_t>(typeExtraPtr->modelId);
        baseOffset = curOffset;
    }

    return false;
}

// 计算 模型属性(模型成员)的信息
int32_t StructLayoutCalculator::CalculateModelLayout(const ModelRecordPtr &modelPtr,
                                                     const std::vector<ModelPropRecordPtr> &props)
{
    if (m_modelLayouts.find(modelPtr->model_id) != m_modelLayouts.end())
    {
        return 0;
    }

    ModelLayout layout;
    layout.model_id = modelPtr->model_id;
    layout.model_name = modelPtr->name_space + "::" + modelPtr->model_name;
    layout.align = ParseAlign(modelPtr->aligh_length);
    layout.total_length = 0;
    layout.props.reserve(props.size());

    uint32_t currentOffset = 0;

    for (const auto &propPtr : props)
    {
        if (!propPtr)
        {
            continue;
        }

        const ModelPropRecord &prop = *propPtr;
        ModelPropLayoutPtr propLayout = std::make_shared<ModelPropLayout>();
        propLayout->prop_name = prop.prop_name;
        propLayout->type_id = prop.type_id;

        TypeExtraPtr typeExtraPtr = std::make_shared<TypeExtra>();
        if (!ParseTypeExtra(prop.type_extra, *typeExtraPtr))
        {
            return -1;
        }
        uint32_t typeAlign = GetTypeAlign(prop.type_id, prop.type_extra, *typeExtraPtr);
        uint32_t typeLength = GetTypeLength(prop.type_id, prop.type_extra, *typeExtraPtr, layout.align);

        uint32_t align = std::min(typeAlign, layout.align);

        if (currentOffset % align != 0)
        {
            currentOffset = ((currentOffset / align) + 1) * align;
        }

        propLayout->offset = currentOffset;
        propLayout->length = typeLength;
        propLayout->align = typeAlign;
        if (!prop.type_extra.empty())
        {
            propLayout->type_extra = typeExtraPtr;
        }

        currentOffset += typeLength;

        layout.props.emplace_back(propLayout);
        layout.prop_name_map.insert({prop.prop_name, propLayout});
    }

    if (currentOffset % layout.align != 0)
    {
        currentOffset = ((currentOffset / layout.align) + 1) * layout.align;
    }

    layout.total_length = currentOffset;

    m_modelLayouts.emplace(layout.model_id, layout);

    return 0;
}

int32_t StructLayoutCalculator::Calculate()
{
    uint32_t ret = 0;
    m_modelLayouts.clear();

    for (const auto &[modelId, modelPtr] : m_cache.models)
    {
        if (!modelPtr)
        {
            continue;
        }
        auto propsIt = m_cache.model_props.find(modelId);
        if (propsIt != m_cache.model_props.end())
        {
            ret = CalculateModelLayout(modelPtr, propsIt->second);
            if (ret != 0)
            {
                fprintf(stderr, "CalculateModelLayout error: modelId=%u\n", modelId);
                return ret;
            }
        }
    }

    ret = CalculateModelInfoLayouts();
    if (ret != 0)
    {
        fprintf(stderr, "CalculateLeafLayouts error\n");
        return ret;
    }

    ret = CalculateVarPropLayouts();
    if (ret != 0)
    {
        fprintf(stderr, "CalculateVarPropLayouts error\n");
        return ret;
    }
    return ret;
}

// 解析模型的叶子节点信息
int32_t StructLayoutCalculator::CalculateModelInfoLayouts()
{
    for (const auto &[rootModelId, infos] : m_cache.model_infos)
    {
        auto itModel = m_modelLayouts.find(rootModelId);
        if (itModel == m_modelLayouts.end())
        {
            fprintf(stderr, "Model not found in m_modelLayouts: model_id=%u\n", rootModelId);
            continue;
        }

        for (const auto &infoPtr : infos)
        {
            if (!infoPtr)
            {
                continue;
            }

            const ModelInfoRecord &info = *infoPtr;
            ModelInfoLayoutPtr outPtr = std::make_shared<ModelInfoLayout>();
            outPtr->parent_model_id = rootModelId;
            outPtr->info_id = info.id;
            outPtr->prop_name = info.prop_name;
            outPtr->type_id = info.type_id;

            VarAttr varAttr;
            if (!ResolvePathInModel(rootModelId, info.prop_name, varAttr))
            {
                fprintf(stderr, "ResolvePathInModel failed: model_id=%u prop_name=%s\n", rootModelId,
                        info.prop_name.c_str());
                continue;
            }
            outPtr->offset = varAttr.offset;
            outPtr->length = varAttr.length;
            outPtr->type_id = varAttr.type_id;
            outPtr->type_extra = varAttr.type_extra;

            // 额外校验：叶子成员在 MDL_INFO 里存了 TYPE_ID（通常是基础类型）。
            // STRING 的长度受 TYPE_EXTRA 影响，这里不强校验，避免误报。
            if (outPtr->length > 0 && info.type_id != 0 && info.type_id != 17)
            {
                auto itType = m_cache.types.find(info.type_id);
                if (itType != m_cache.types.end() && itType->second && itType->second->length > 0 &&
                    itType->second->length != outPtr->length)
                {
                    fprintf(
                        stderr,
                        "[WARN] Leaf length mismatch: model_id=%u prop=%s info.type_id=%u expect_len=%u calc_len=%u\n",
                        rootModelId, info.prop_name.c_str(), info.type_id, itType->second->length, outPtr->length);
                }
            }

            m_modelLayouts[rootModelId].infos.emplace_back(outPtr);
            m_modelLayouts[rootModelId].info_name_map.insert({info.prop_name, outPtr});
        }
    }

    return 0;
}

// ============================================================
// 在模型布局中解析成员路径，得到 offset 和 length
// path 格式: "MEMBER" 或 "MEMBER.SUB" 或 "ARR[0].MEMBER" 等
// ============================================================
bool StructLayoutCalculator::CalcuatePathInModel(uint32_t modelId, const std::string &path, VarAttr &varAttr)
{
    // 叶子节点
    const ModelInfoLayoutPtr infoLayoutPtr = FindModelInfoLayoutByName(m_modelLayouts, modelId, path);
    if (nullptr != infoLayoutPtr)
    {
        varAttr.offset = infoLayoutPtr->offset;
        varAttr.length = infoLayoutPtr->length;
        varAttr.type_id = infoLayoutPtr->type_id;
        varAttr.type_extra = infoLayoutPtr->type_extra;
        return true;
    }
    // 上层节点
    // fprintf(stderr, "member not found: modelId=%u path=%s\n", modelId, path.c_str());
    return ResolvePathInModel(modelId, path, varAttr);
}

// 所有变量的类型、长度
int32_t StructLayoutCalculator::CalculateVarLayouts()
{
    for (const auto &[tagId, varPtr] : m_cache.vars)
    {
        if (!varPtr)
            continue;
        VarLayoutPtr varLayoutPtr = std::make_shared<VarLayout>();
        VarLayout &varLayout = *varLayoutPtr;
        varLayout.tag_id = tagId;
        varLayout.tag_name = varPtr->name_space + "::" + varPtr->tag_name;
        varLayout.type_id = varPtr->type_id;

        uint32_t typeId = varPtr->type_id;

        if (!varPtr->type_extra.empty())
        {
            varLayout.type_extra = std::make_shared<TypeExtra>();
            if (!ParseTypeExtra(varPtr->type_extra, *(varLayout.type_extra)))
            {
                fprintf(stderr, "ParseTypeExtra failed: tag_id=%u\n", tagId);
                continue;
            }
        }
        const TypeExtraPtr &typeExtraPtr = varLayout.type_extra;

        // 计算变量的 total_length
        if (varPtr->type_extra.empty())
        {
            // 基础类型（无 TYPE_EXTRA）
            auto typeIt = m_cache.types.find(typeId);
            if (typeIt != m_cache.types.end() && typeIt->second)
            {
                varLayout.total_length = typeIt->second->length;
            }
            else
            {
                fprintf(stderr, "type not found: tag_id=%u, type_id=%u\n", tagId, typeId);
                continue;
            }
        }
        else if (typeId == 17)
        {
            // STRING
            varLayout.total_length = typeExtraPtr->strLength + 2;
        }
        else if (typeId == 27 && typeExtraPtr->modelId > 0)
        {
            // UDT
            auto layoutIt = m_modelLayouts.find(typeExtraPtr->modelId);
            if (layoutIt != m_modelLayouts.end())
            {
                varLayout.total_length = layoutIt->second.total_length;
            }
            else
            {
                fprintf(stderr, "modelId not found: tag_id=%u, modelId=%u\n", tagId, typeExtraPtr->modelId);
                continue;
            }
        }
        else if (typeId == 26 && !typeExtraPtr->range.empty())
        {
            // ARRAY
            uint32_t arrayCount = typeExtraPtr->range[0].end - typeExtraPtr->range[0].start + 1;
            uint32_t eleSize = 0;
            if (GetArrayElementSize(m_cache.types_by_name, m_modelLayouts, *typeExtraPtr, eleSize))
                varLayout.total_length = arrayCount * eleSize;
        }

        m_varLayouts.emplace(varLayoutPtr->tag_id, varLayoutPtr);
        m_varLayoutsByName.emplace(varLayoutPtr->tag_name, varLayoutPtr);
    }

    return 0;
}
// ============================================================
// 变量基础表叶子成员布局计算
// ============================================================
int32_t StructLayoutCalculator::CalculateVarPropLayouts()
{
    m_varLayouts.clear();
    m_varLayoutsByName.clear();
    m_tagAttrMap.clear();

    // ----------------------------------------------------------
    // 第一步：为每个变量创建 VarLayout 并计算 total_length
    // ----------------------------------------------------------
    CalculateVarLayouts();
    // ----------------------------------------------------------
    // 第二步：遍历所有 var_prop，计算每个叶子的 offset/length
    // ----------------------------------------------------------
    for (const auto &[tagId, var_pros] : m_cache.var_props)
    {

        auto varIt = m_varLayouts.find(tagId);
        if (varIt == m_varLayouts.end())
        {
            fprintf(stderr, "var not found: tag_id=%u\n", tagId);
            continue;
        }
        VarLayout &var = *(varIt->second);
        for (auto vpPtr : var_pros)
        {
            CalculateVarPropLayoutDetail(var, vpPtr->prop_name, vpPtr->prop_id);
        }
    }

    return 0;
}

// 计算变量的基础信息:propName可以是变量本身，也可以是一部分
int32_t StructLayoutCalculator::CalculateVarPropLayoutDetail(VarLayout &var, const std::string &propName,
                                                             uint32_t propId)
{
    VarPropLayoutPtr outPtr = std::make_shared<VarPropLayout>();
    outPtr->tag_id = var.tag_id;
    outPtr->prop_id = propId;
    outPtr->prop_name = propName;

    bool ok = false;
    const auto varTypeId = var.type_id;
    // ---- udt 本身, array 本身,基础类型/STRING 类型 单点 ----
    if (var.tag_name == propName)
    {
        // PROP_NAME 格式:  "NAMESPACE::TAG_NAME"
        outPtr->type_id = var.type_id;
        outPtr->type_extra = var.type_extra;
        outPtr->offset = 0;
        outPtr->length = var.total_length;
        ok = true;
    }
    // ---- UDT 类型：在模型布局中解析成员路径 ----
    else if (27 == varTypeId && var.type_extra->modelId > 0)
    {
        // PROP_NAME 格式: "NAMESPACE::TAG_NAME.MEMBER_PATH"
        std::string tagNameDot = var.tag_name + ".";
        std::string memberPath;
        if (propName.size() > tagNameDot.size() && propName.substr(0, tagNameDot.size()) == tagNameDot)
        {
            memberPath = propName.substr(tagNameDot.size());

            VarAttr varAttr;
            if (CalcuatePathInModel(var.type_extra->modelId, memberPath, varAttr))
            {
                outPtr->type_id = varAttr.type_id;
                outPtr->type_extra = varAttr.type_extra;
                outPtr->offset = varAttr.offset;
                outPtr->length = varAttr.length;

                ok = true;
            }
        }

        if (!ok)
        {
            fprintf(stderr, "UDT resolve failed: prop_name=%s\n", propName.c_str());
        }
    }
    // ---- ARRAY 类型：从 PROP_NAME 提取数组索引 ----
    else if (26 == varTypeId)
    {
        // PROP_NAME 格式: "NAMESPACE::TAG_NAME[idx]" 、 "NAMESPACE::TAG_NAME[idx].MEMBER"
        std::string suffix;
        if (propName.size() > var.tag_name.size() && propName.substr(0, var.tag_name.size()) == var.tag_name)
        {
            suffix = propName.substr(var.tag_name.size());

            if (!suffix.empty() && suffix[0] == '[')
            {
                size_t rb = suffix.find(']');
                if (rb != std::string::npos && !var.type_extra->range.empty())
                {
                    std::string idxStr = suffix.substr(1, rb - 1);
                    int index = 0;
                    try
                    {
                        index = std::stoi(idxStr);
                    }
                    catch (...)
                    {
                        fprintf(stderr, "bad index: prop_name=%s\n", propName.c_str());
                        return -1;
                    }

                    const Range &r0 = var.type_extra->range[0];
                    uint32_t eleSize = 0;
                    if (GetArrayElementSize(m_cache.types_by_name, m_modelLayouts, *var.type_extra, eleSize) &&
                        eleSize > 0)
                    {
                        uint32_t baseOffset = static_cast<uint32_t>(index - r0.start) * eleSize;

                        // ']' 后面是否有 ".member" 路径（数组元素是模型的情况）
                        std::string remainPath;
                        if (rb + 1 < suffix.size() && suffix[rb + 1] == '.')
                        {
                            remainPath = suffix.substr(rb + 2);
                        }

                        if (remainPath.empty())
                        {
                            // 数组元素就是叶子
                            uint32_t eleTypeId = 0;
                            TypeExtra eleTypeExtra;
                            if (!GetArrayElementType(m_cache.types_by_name, m_modelLayouts, *var.type_extra, eleTypeId,
                                                     eleTypeExtra))
                            {
                                fprintf(stderr, "element type invalid: propName=%s\n", propName.c_str());
                                return false;
                            }
                            outPtr->type_id = eleTypeId;
                            if (eleTypeExtra.modelId > 0 || eleTypeExtra.strLength > 0)
                            {
                                outPtr->type_extra = std::make_shared<TypeExtra>(eleTypeExtra);
                            }
                            outPtr->offset = baseOffset;
                            outPtr->length = eleSize;
                            ok = true;
                        }
                        else if (var.type_extra->modelId > 0)
                        {
                            // 数组元素是模型，需要继续解析成员路径
                            uint32_t memberOffset = 0, memberLength = 0;
                            VarAttr varAttr;
                            if (CalcuatePathInModel(var.type_extra->modelId, remainPath, varAttr))
                            {
                                outPtr->type_id = varAttr.type_id;
                                outPtr->type_extra = varAttr.type_extra;
                                outPtr->offset = baseOffset + varAttr.offset;
                                outPtr->length = varAttr.offset;
                                ok = true;
                            }
                        }
                    }
                }
            }
        }
        if (!ok)
        {
            fprintf(stderr, "ARRAY resolve failed: prop_name=%s\n", propName.c_str());
        }
    }

    if (ok)
    {
        var.props.emplace_back(outPtr);
        var.prop_name_map.insert({propName, outPtr});

        m_tagAttrMap.insert({propName, outPtr}); // TODO(wuzheqiang):这个name 是包含域名且唯一?
    }
    return 0;
}

VarPropLayoutPtr StructLayoutCalculator::GetTagAttr(const std::string &tagName)
{
    // 散点或者叶子节点，直接返回
    auto iter = m_tagAttrMap.find(tagName);
    if (iter != m_tagAttrMap.end())
    {
        return iter->second;
    }

    // 变量名称 STD::TagName.member  STD::TagName[10].member
    std::string objName = tagName;
    size_t nsPos = tagName.find("::");
    if (nsPos == std::string::npos)
    {
        return nullptr;
    }
    size_t tagPos = tagName.find_first_of(".[", nsPos);
    if (tagPos != std::string::npos)
    {
        objName = tagName.substr(0, tagPos);
    }

    // 查找变量信息
    auto iterVar = m_varLayoutsByName.find(objName);
    if (iterVar == m_varLayoutsByName.end())
    {
        printf("GetTagAttr:objName not found:objName=%s, tagName=%s\n", objName.c_str(), tagName.c_str());
        return nullptr;
    }

    VarLayout &var = *(iterVar->second);
    CalculateVarPropLayoutDetail(var, tagName);

    iter = m_tagAttrMap.find(tagName);
    if (iter != m_tagAttrMap.end())
    {
        return iter->second;
    }

    return nullptr;
}
} // namespace data_attr