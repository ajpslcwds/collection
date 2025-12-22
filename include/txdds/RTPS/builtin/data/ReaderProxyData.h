#ifndef TXDDS_RTPS_READERPROXYDATA_H
#define TXDDS_RTPS_READERPROXYDATA_H
#include "txdds/RTPS/common/Guid.h"
#include "txdds/RTPS/common/InstanceHandle.h"
#include "txdds/RTPS/common/LocatorList_t.h"
#include "txdds/RTPS/common/RTPSEntityTypes.h"

namespace BaoSky::rtps
{
    class ReaderProxyData
    {
    public:
        ReaderProxyData() = default;
        virtual ~ReaderProxyData() = default;
        inline void SetGuid(const GUID &guid)
        {
            mRemoteReaderGuid = guid;
        }
        inline void SetKey(const GUID &key)
        {
            mReaderKey = key;
        }
        inline void SetParticipantKey(const GUID &key)
        {
            mParticipantKey = key;
        }
        inline void SetUnicastLocator(const LocatorList_t &locator)
        {
            unicastLocatorList = locator;
        }
        inline void SetMulticastLocator(const LocatorList_t &locator)
        {
            multicastLocatorList = locator;
        }
        inline void SetTopicName(const std::string &topicName)
        {
            mTopicName = topicName;
        }
        inline void SetTypeName(const std::string &typeName)
        {
            mTypeName = typeName;
        }
        inline void SetTopicKind(const TopicKind_t &topicKind)
        {
            mTopicKind = topicKind;
        }
        inline void SetExpectsInlineQos(bool expectsInlineQos)
        {
            mExpectsInlineQos = expectsInlineQos;
        }
        inline void SetGroupGuid(const GUID &guid)
        {
            mRemoteGroupGuid = guid;
        }
        inline void SetReliable()
        {
            mIsReliable = true;
        }
        inline std::string GetTopicName() const
        {
            return mTopicName;
        }
        inline std::string GetTypeName() const
        {
            return mTypeName;
        }
        inline GUID &GetGuid()
        {
            return mRemoteReaderGuid;
        }
        inline LocatorList_t &GetUnicastLocator()
        {
            return unicastLocatorList;
        }
        inline LocatorList_t &GetMulticastLocator()
        {
            return multicastLocatorList;
        }
        inline TopicKind_t GetTopicKind() const
        {
            return mTopicKind;
        }
        inline InstanceHandle GetParticipantKey() const
        {
            return mParticipantKey;
        }
        inline InstanceHandle GetReaderKey() const
        {
            return mReaderKey;
        }
        inline bool GetExpectsInlineQos() const
        {
            return mExpectsInlineQos;
        }
        inline bool GetReliable()
        {
            return mIsReliable;
        }

    private:
        GUID mRemoteReaderGuid;
        GUID mRemoteGroupGuid;
        LocatorList_t unicastLocatorList;
        LocatorList_t multicastLocatorList;
        std::string mTopicName;
        std::string mTypeName;
        TopicKind_t mTopicKind;
        InstanceHandle mReaderKey;
        InstanceHandle mParticipantKey;
        bool mExpectsInlineQos;
        bool mIsReliable = false;
    };
}

#endif