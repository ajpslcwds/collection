/*
 * @Author       : songwenguang 563734@baosight.com
 * @Date         : 2024-07-19 17:51:44
 * @FilePath: /TXDDS/include/RTPS/builtin/discovery/participant/PDPListeners.h
 * @Copyright (c) 2023 by BAOSIGHT, All Rights Reserved.
 * @Description  :
 */
#ifndef TXDDS_RTPS_PDPListeners_H
#define TXDDS_RTPS_PDPListeners_H
#include "txdds/RTPS/reader/IReaderListener.h"
#include "txdds/RTPS/builtin/discovery/participant/PDP.h"
#include "txdds/RTPS/history/CacheChange.h"

namespace BaoSky::rtps
{
    class PDPListener : public IReaderListener
    {
    public:
        PDPListener(PDP *pdp);

        virtual ~PDPListener() override {}

        virtual void OnReaderMatched(IReader *reader, const MatchingInfo &info) override;

        /*
         * @description: 新的CacheChange到达builtin reader后配置对应的DiscoveredParticipantData
         * @param {Reader} *reader
         * @param {CacheChange} *change
         * @return {*}
         */
        virtual void OnNewCacheChangeAdded(IReader *reader, const CacheChange *const change) override;

        virtual void OnWriterDiscovery(
            IReader *reader,
            WriterDiscoveryInfo::DISCOVERY_STATUS reason,
            const GUID &writer_guid,
            const WriterProxyData *writer_info) override;

        virtual void OnDataAvailable(
            IReader *reader,
            const GUID &writer_guid,
            const SequenceNumber &first_sequence,
            const SequenceNumber &last_sequence,
            bool &should_notify_individual_changes) override;
        /*
         * @description: 处理配置对应的DiscoveredParticipantData
         * @param {DiscoveredParticipantData} *oldData
         * @param {DiscoveredParticipantData} *newData
         * @param {GUID} &guid
         * @param {Reader} *reader
         * @param {unique_lock<std::recursive_mutex>} &lock
         * @return {*}
         */
        virtual void ProcessDiscoveredParticipantData(std::shared_ptr<DiscoveredParticipantData> oldData, DiscoveredParticipantData *newData,
                                                      const GUID &guid, IReader *reader, std::unique_lock<std::recursive_mutex> &lock);

    private:
        PDP *mPDP;
        DiscoveredParticipantData mDiscoveredParticipantData;
        virtual bool ReadGuidFromChange(CacheChange *cacheChange, GUID &guid, CDRMessage &cdrMsg);
    };
}

#endif