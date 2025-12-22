/*
 * @Author: songwenguang 563734@baosight.com
 * @Date: 2025-04-06 18:21:21
 * @FilePath: /TXDDS/include/DCPS/dynamicType/DynamicTopicDataType.h
 * @Copyright (c) 2025 by BAOSIGHT, All Rights Reserved.
 * @Description:
 */
#pragma once
#include <map>

#include "txdds/DCPS/common/ReturnCode.h"
#include "txdds/DCPS/dynamicType/DynamicType.h"
#include "txdds/DCPS/topic/TopicDataType.h"
using ReturnCode = BaoSky::dds::ReturnCode;
namespace BaoSky::dds
{
    class DynamicTopicDataType : public virtual TopicDataType
    {
    public:
        DynamicTopicDataType(std::shared_ptr<DynamicType> dynamicType);
        ~DynamicTopicDataType();
        DynamicTopicDataType(const DynamicTopicDataType &) = delete;
        DynamicTopicDataType &operator=(const DynamicTopicDataType &) = delete;
        virtual bool Serialize(void *data, BaoSky::rtps::SerializedPayload *payload) override;
        virtual bool Deserialize(BaoSky::rtps::SerializedPayload *payload, void *data) override;
        virtual void *CreateData() override;
        virtual void DeleteData(void *data) override;
        virtual std::function<uint32_t()> GetSerializedSizeProvider(void *data) override;
        virtual bool GetKey(void *data, BaoSky::rtps::InstanceHandle *ihandle, bool force_md5 = false) override;
        virtual ReturnCode RegisterTypeObject(TypeObjectParameter &typeObjectParameter, TypeIdentifierParameter &typeIdentifierParameter);

    private:
        virtual uint32_t CaculateDynamicTopicDataTypeSize();
        std::shared_ptr<DynamicType> mDynamicType;
    };
}