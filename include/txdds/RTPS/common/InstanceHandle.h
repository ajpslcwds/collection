/*
 * @Author      : wengjiqing wengjiqing@baosight.com
 * @Date        : 2024-09-24 16:23:48
 * @FilePath: /TXDDS/include/RTPS/common/InstanceHandle.h
 * Copyright (c) 2024 by BAOSIGHT, All Rights Reserved.
 * @Description :
 */
#ifndef TXDDS_RTPS_INSTANCEHANDLE_H
#define TXDDS_RTPS_INSTANCEHANDLE_H

#include <array>
#include "txdds/RTPS/common/Guid.h"

namespace BaoSky::rtps
{
    using KeyHash_t = std::array<unsigned char, 16>;
    // InstanceHandle
    struct InstanceHandleValue
    {
        operator octet *() noexcept
        {
            mIsSet = true;
            return mValue.data();
        }
        operator const octet *() const noexcept
        {
            return mValue.data();
        }
        bool IsSet() const noexcept
        {
            return mIsSet;
        }
        void clear() noexcept
        {
            mValue.fill(0);
            mIsSet = false;
        }
        bool operator==(
            const InstanceHandleValue &other) const noexcept
        {
            return (mIsSet == other.mIsSet) && (mValue == other.mValue);
        }
        bool operator<(
            const InstanceHandleValue &other) const noexcept
        {
            if (mIsSet)
            {
                return other.mIsSet && mValue < other.mValue;
            }

            return other.mIsSet;
        }

        KeyHash_t mValue{};
        bool mIsSet = false;
    };
    struct InstanceHandle
    {

        InstanceHandle() noexcept = default;
        InstanceHandle(const InstanceHandle &instanceHandle) noexcept = default;
        InstanceHandle(const GUID &guid) noexcept
        {
            *this = guid;
        }
        InstanceHandle &operator=(const InstanceHandle &instanceHandle) noexcept = default;
        // todo: InstanceHandle reload
        InstanceHandle &operator=(
            const GUID &guid) noexcept
        {
            octet *dst = mHandleValue;
            memcpy(dst, guid.mGuidPrefix.mValue, 12);
            memcpy(&dst[12], guid.mEntityId.mValue, 4);
            return *this;
        }

        bool IsSet() const noexcept
        {
            return mHandleValue.IsSet();
        }
        void Clear() noexcept
        {
            mHandleValue.clear();
        }
        InstanceHandleValue mHandleValue;
    };
    const InstanceHandle INSTANCEHANDLE_UNKNOWN;
    /**
     * @brief Comparison operator: checks if a InstanceHandle is less than another.
     *
     * @param h1 First InstanceHandle to compare.
     * @param h2 Second InstanceHandle to compare.
     * @return True if the first InstanceHandle is less than the second.
     */
    inline bool operator<(
        const InstanceHandle &h1,
        const InstanceHandle &h2) noexcept
    {
        return h1.mHandleValue < h2.mHandleValue;
    }

    inline bool operator==(const InstanceHandle &handle1, const InstanceHandle &handle2) noexcept
    {
        return handle1.mHandleValue == handle2.mHandleValue;
    }

    inline bool operator!=(const InstanceHandle &handle1, const InstanceHandle &handle2) noexcept
    {
        return handle1.IsSet() != handle2.IsSet() || handle1.mHandleValue != handle2.mHandleValue;
    }

    inline void iHandle2GUID(
        GUID &guid,
        const InstanceHandle &ihandle) noexcept
    {
        const octet *value = ihandle.mHandleValue.mValue.data();
        memcpy(guid.mGuidPrefix.mValue, value, 12);
        memcpy(guid.mEntityId.mValue, &value[12], 4);
    }
}

#endif