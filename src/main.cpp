#include "struct_layout.h"

#include <cstdio>
#include <string>
#include <vector>

void TestGetAttr();
// ----------------------------------------------------------
// 数据库连接参数（按需修改）
// ----------------------------------------------------------
const std::string host = "127.0.0.1";
const uint16_t port = 3306;
const std::string database = "dsf";   // <-- 修改为实际库名
const std::string user = "root";      // <-- 修改为实际用户名
const std::string password = "mysql"; // <-- 修改为实际密码

int main(int argc, char *argv[])
{
    try
    {
        data_attr::DbConfig db_config = {SA_MySQL_Client, host, port, database, user, password};
        auto &calculator = data_attr::StructLayoutCalculator::GetInstance();
        int iRet = calculator.Init(db_config);
        if (iRet != 0)
        {
            printf("[ERROR] calculator Init failed\n");
            return iRet;
        }

        // ----------------------------------------------------------
        // 计算模型 total_length
        // ----------------------------------------------------------
        const auto &results = calculator.GetModelLayouts();
        for (const auto &[id, layout] : results)
        {
            printf("\n[Model] %s (id=%u, align=%u, total_size=%u)\n", layout.model_name.c_str(), layout.model_id,
                   layout.align, layout.total_length);
            for (const auto &prop : layout.props)
            {
                printf("  %s: offset=%u, length=%u, align=%u, type_id=%u, TypeExtra:%s\n", prop->prop_name.c_str(),
                       prop->offset, prop->length, prop->align, prop->type_id,
                       DisplayTypeExtraPtr(prop->type_extra).c_str());
            }

            // ----------------------------------------------------------
            // 模型散点（叶子成员）offset/length
            // ----------------------------------------------------------

            const auto &leaves = layout.infos;
            printf("\n[ModelInfo] %s (id=%u), leaf_count=%zu\n", layout.model_name.c_str(), layout.model_id,
                   leaves.size());
            for (const auto &leaf : leaves)
            {
                printf("  %s: offset=%u, length=%u, type_id=%u, TypeExtra:%s\n", leaf->prop_name.c_str(), leaf->offset,
                       leaf->length, leaf->type_id, DisplayTypeExtraPtr(leaf->type_extra).c_str());
            }

            // const auto &leaves_map = layout.info_name_map;
            // printf("\n[ModelInfo] %s (id=%u), leaf_count=%zu\n",
            //        layout.model_name.c_str(), layout.model_id, leaves_map.size());
            // for (const auto &[prop_name, leaf] : leaves_map)
            // {
            //     printf("  %s: offset=%u, length=%u\n", prop_name.c_str(), leaf->offset, leaf->length);
            // }
        }

        // ----------------------------------------------------------
        // 变量基础表（叶子成员）offset/length
        // ----------------------------------------------------------
        const auto &varLayouts = calculator.GetVarLayouts();
        printf("\n[INFO] Variable layouts: %zu vars", varLayouts.size());
        for (const auto &[tagId, varLayout] : varLayouts)
        {
            printf("\n[Var] %s (tag_id=%u, type_id=%u, TypeExtra:%s, total_length=%u, prop_count=%zu)\n",
                   varLayout->tag_name.c_str(), varLayout->tag_id, varLayout->type_id,
                   DisplayTypeExtraPtr(varLayout->type_extra).c_str(), varLayout->total_length,
                   varLayout->props.size());
            for (const auto &vp : varLayout->props)
            {
                printf("  %s: offset=%u, length=%u, type_id=%u, TypeExtra:%s\n", vp->prop_name.c_str(), vp->offset,
                       vp->length, vp->type_id, DisplayTypeExtraPtr(vp->type_extra).c_str());
            }
        }

        printf("\n[INFO] Done.\n");

        TestGetAttr();
    }
    catch (std::exception &e)
    {
        fprintf(stderr, "[ERROR] %s\n", e.what());
        return 1;
    }
    catch (...)
    {
        fprintf(stderr, "[ERROR] Unknown exception\n");
        return 1;
    }

    return 0;
}

void TestGetAttr()
{
    std::vector<std::string> tagNames = {"STD::DS_DMODEL2.ARR_U1V1[0].STR2",
                                         "STD::DS_DMODEL2.ARR_U1V1[0]",
                                         "STD::DS_DMODEL2.ARR_U1V1[0]",
                                         "STD::DS_DMODEL2.ARR_U1V1",
                                         "STD::DS_DMODEL2",
                                         "STD::DS_ARRAY_MODEL2[1].ARR_U1V1[0].STR2",
                                         "STD::DS_ARRAY_MODEL2[1].ARR_U1V1[0]",
                                         "STD::DS_ARRAY_MODEL2[1].ARR_U1V1",
                                         "STD::DS_ARRAY_MODEL2[1]",
                                         "STD::DS_ARRAY_MODEL2",
                                         "STD::DS_PERF_INT",
                                         "STD::DS_PERF_INT[12]"};
    auto &calculator = data_attr::StructLayoutCalculator::GetInstance();
    for (const auto &tagName : tagNames)
    {
        data_attr::VarPropLayoutPtr attr = calculator.GetTagAttr(tagName);
        if (nullptr == attr)
        {
            printf("GetTagAttr tagName:%s, attr is null\n", tagName.c_str());
        }
        else
        {
            printf("GetTagAttr tagName:%s, attr->prop_name:%s, attr->type_id:%u, attr->type_extra:%s, attr->offset:%u, "
                   "attr->length:%u\n",
                   tagName.c_str(), attr->prop_name.c_str(), attr->type_id,
                   DisplayTypeExtraPtr(attr->type_extra).c_str(), attr->offset, attr->length);
        }
    }
}
