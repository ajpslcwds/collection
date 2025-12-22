#ifndef TXDDS_RTPS_IREADERLISTENER_H
#define TXDDS_RTPS_IREADERLISTENER_H

#include "txdds/RTPS/common/Guid.h"
#include "txdds/RTPS/common/MatchingInfo.h"
#include "txdds/RTPS/builtin/data/WriterProxyData.h"
#include "txdds/RTPS/writer/WriterDiscoveryInfo.h"
#include "txdds/RTPS/common/SequenceNumber.h"

namespace BaoSky::rtps
{
    class IReader;
    class CacheChange;

    class IReaderListener
    {
    public:
        virtual ~IReaderListener() {}

        /**
         * @Description : 当 Reader 与 Writer 匹配时触发
         * @param        {Reader} *reader
         * @param        {MatchingInfo} &info
         * @return       {*}
         */
        virtual void OnReaderMatched(IReader *reader, const MatchingInfo &info)
        {
            (void)reader;
            (void)info;
        }

        /**
         * @Description : 当新的 CacheChange 被添加到 ReaderHistory 时触发
         * @param        {Reader} *reader
         * @param        {CacheChange} *change
         * @return       {*}
         */
        virtual void OnNewCacheChangeAdded(IReader *reader, const CacheChange *const change)
        {
            (void)reader;
            (void)change;
        }

        /**
         * @Description : 当和 Writer match unmatch 时触发
         * @param        {Reader} *reader
         * @param        {DISCOVERY_STATUS} reason
         * @param        {GUID} &writer_guid
         * @param        {WriterProxyData} *writer_info
         * @return       {*}
         */
        virtual void OnWriterDiscovery(
            IReader *reader,
            WriterDiscoveryInfo::DISCOVERY_STATUS reason,
            const GUID &writer_guid,
            const WriterProxyData *writer_info)
        {
            (void)reader;
            (void)reason;
            (void)writer_guid;
            (void)writer_info;
        }

        /**
         * @Description : 当新的 CacheChange 对用户可见时触发
         * @param        {Reader} *reader
         * @param        {GUID} &writer_guid
         * @param        {SequenceNumber} &first_sequence
         * @param        {SequenceNumber} &last_sequence
         * @param        {bool} &should_notify_individual_changes
         * @return       {*}
         */
        virtual void OnDataAvailable(
            IReader *reader,
            const GUID &writer_guid,
            const SequenceNumber &first_sequence,
            const SequenceNumber &last_sequence,
            bool &should_notify_individual_changes)
        {
            (void)reader;
            (void)writer_guid;
            (void)first_sequence;
            (void)last_sequence;
            should_notify_individual_changes = true;
        }
    };
}

#endif