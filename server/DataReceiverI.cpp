#include "DataReceiverI.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

void DataReceiverI::sendData(const DSF::DataUnitSeq &dataSeq, const Ice::Current &)
{
    json allData;

    // 如果 data.json 已存在则读入原有内容
    std::ifstream in("data.json");
    if (in.good())
    {
        try
        {
            in >> allData;
        }
        catch (...)
        {
            allData = json::array(); // 文件坏了就清空
        }
    }
    in.close();

    for (const auto &data : dataSeq)
    {
        json entry;
        entry["name"] = data->strName;
        entry["time"] = data->lTime;

        switch (data->eType)
        {
        case DSF::ValueType::Decimal:
            entry["type"] = "Decimal";
            if (data->dValue)
            {
                entry["value"] = *data->dValue;
            }
            break;
        case DSF::ValueType::Integer:
            entry["type"] = "Integer";
            if (data->lValue)
            {
                entry["value"] = *data->lValue;
            }
            break;
        case DSF::ValueType::Bool:
            entry["type"] = "Bool";
            if (data->bValue)
            {
                entry["value"] = *data->bValue;
            }
            break;
        case DSF::ValueType::Text:
            entry["type"] = "Text";
            if (data->strValue)
            {
                entry["value"] = *data->strValue;
            }
            break;
        }

        allData.push_back(entry);
    }

    std::ofstream out("data.json");
    out << allData.dump(4) << std::endl;
    std::cout << "Batch data received and written. Count: " << dataSeq.size() << std::endl;
}
