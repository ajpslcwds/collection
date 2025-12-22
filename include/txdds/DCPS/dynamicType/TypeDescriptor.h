/*
 * @Author: songwenguang 563734@baosight.com
 * @Date: 2025-03-13 15:59:03
 * @FilePath: /TXDDS/include/DCPS/dynamicType/TypeDescriptor.h
 * @Copyright (c) 2025 by BAOSIGHT, All Rights Reserved.
 * @Description:
 */

#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <memory>

#include "txdds/DCPS/dynamicType/CommonType.h"
#include "txdds/DCPS/common/ReturnCode.h"
using ReturnCode = BaoSky::dds::ReturnCode;

namespace BaoSky::dds
{
    class DynamicType;

    class TypeDescriptor
    {
    public:
        TypeDescriptor();
        TypeDescriptor(TypeKind kind);
        TypeDescriptor(const TypeDescriptor &) = delete;
        TypeDescriptor &operator=(const TypeDescriptor &) = delete;
        ReturnCode CopyFromOtherType(const TypeDescriptor *other);
        bool IsSameWithOtherType(const TypeDescriptor *other);
        bool IsTypeConsistent();
        std::string GetDynTypeName();
        std::vector<uint32_t> &GetDynTypeBound();
        
        void SetDynTypeElementType(std::shared_ptr<DynamicType> dynamicType);
        TypeKind GetDynTypeKind() const;
        void SetDynTypeKind(TypeKind kind);
        void SetDynTypeName(std::string name);
        std::shared_ptr<DynamicType> GetDynTypeElementType();
        void SetDynTypeKeyType(std::shared_ptr<DynamicType> dynamicType);
        std::shared_ptr<DynamicType> GetDynTypeKeyType();
        void SetDynTypeBaseType(std::shared_ptr<DynamicType> dynamicType);

    private:
        TypeKind mDynTypeKind;
        std::string mDynTypeName;
        std::shared_ptr<DynamicType> mDynTypeBaseType;
        std::vector<uint32_t> mDynTypeBound;
        std::shared_ptr<DynamicType> mDynTypeElementType;
        std::shared_ptr<DynamicType> mDynTypeKeyType;
    };
}