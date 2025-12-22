#ifndef TXDDS_RTPS_IENTITY_H
#define TXDDS_RTPS_IENTITY_H

#include "txdds/RTPS/common/Guid.h"

#include <mutex>
#include <atomic>

namespace BaoSky::rtps
{
    class IEntity
    {
    public:
        IEntity(const GUID &guid) : mGUID(guid), mIsInitial(false) {}

        virtual ~IEntity() {}

        virtual bool Initial() = 0;

        virtual bool Terminate() = 0;

        virtual GUID GetGuid() { return mGUID; }

        virtual std::mutex &GetMutex() { return mMtx; }

    protected:
        GUID mGUID;

        std::atomic_bool mIsInitial;

        std::mutex mMtx;
    };
}

#endif