#ifndef TXDDS_RTPS_Guid_H
#define TXDDS_RTPS_Guid_H

#include "txdds/RTPS/common/GuidPrefix.h"
#include "txdds/RTPS/common/EntityId.h"

#include <cstdint>
#include <cstring>
#include <sstream>

namespace BaoSky::rtps
{
    // GUID
    struct GUID
    {
        GuidPrefix mGuidPrefix;
        EntityId mEntityId;
        GUID()
        {
        }
        GUID(const GuidPrefix &prefix, const EntityId &entityId)
            : mGuidPrefix(prefix), mEntityId(entityId)
        {
        }
        GUID(const GuidPrefix &prefix, uint32_t entityId)
            : mGuidPrefix(prefix), mEntityId(entityId)
        {
        }
        static GUID Unknown()
        {
            return GUID();
        }
        bool IsOnSameHost(const GUID &guid)
        {
            return mGuidPrefix.IsOnSameHost(guid.mGuidPrefix);
        }
        bool IsOnSameProcess(const GUID &guid)
        {
            return mGuidPrefix.IsOnSameProcess(guid.mGuidPrefix);
        }
        // todo: is builtin
        bool IsBuiltin() const
        {
            return mEntityId.mValue[3] >= 0xc0;
        }
    };
    const GUID GUID_UNKNOWN;

    inline bool operator==(
        const GUID &g1,
        const GUID &g2)
    {
        if (g1.mGuidPrefix == g2.mGuidPrefix && g1.mEntityId == g2.mEntityId)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    inline bool operator!=(
        const GUID &g1,
        const GUID &g2)
    {
        if (g1.mGuidPrefix != g2.mGuidPrefix || g1.mEntityId != g2.mEntityId)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    inline bool operator<(
        const GUID &g1,
        const GUID &g2)
    {
        auto prefix_cmp = GuidPrefix::cmp(g1.mGuidPrefix, g2.mGuidPrefix);
        if (prefix_cmp < 0)
        {
            return true;
        }
        else if (prefix_cmp > 0)
        {
            return false;
        }
        else
        {
            return g1.mEntityId < g2.mEntityId;
        }
    }

    inline std::ostream &operator<<(
        std::ostream &output,
        const GUID &guid)
    {
        if (guid != GUID_UNKNOWN)
        {
            output << guid.mGuidPrefix << "|" << guid.mEntityId;
        }
        else
        {
            output << "|GUID UNKNOWN|";
        }
        return output;
    }
}

#endif