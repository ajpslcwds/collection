/*
 * @Author       : songwenguang 563734@baosight.com
 * @Date         : 2024-07-19 17:42:59
 * @FilePath: /TXDDS/include/RTPS/builtin/discovery/endpoint/EDP.h
 * @Copyright (c) 2023 by BAOSIGHT, All Rights Reserved.
 * @Description  :
 */
#ifndef TXDDS_RTPS_EDP_H
#define TXDDS_RTPS_EDP_H
#include "txdds/RTPS/builtin/discovery/participant/PDP.h"
#include "txdds/RTPS/reader/IStatefulReader.h"
#include "txdds/RTPS/writer/IStatefulWriter.h"
#include "txdds/RTPS/attributes/TopicAttributes.h"
#include "txdds/DCPS/builtin/topic/PublicationBuiltinTopicData.h"
#include "txdds/DCPS/builtin/topic/SubscriptionBuiltinTopicData.h"
#include "txdds/RTPS/builtin/data/DiscoveredWriterData.h"
#include "txdds/RTPS/builtin/data/DiscoveredReaderData.h"
#include "txdds/RTPS/builtin/data/DiscoveredParticipantData.h"

namespace BaoSky::rtps
{
    class BuiltinAttributes;
    class PDP;
    class IParticipant;

    class EDP
    {
    public:
        /*
         * @description: EDP的构造函数
         * @param {PDP} *pdp
         * @param {IParticipant} *participantImpl
         * @return {*}
         */
        EDP(PDP *pdp, IParticipant *participant);
        virtual ~EDP();
        virtual bool InitEDP(const BuiltinAttributes &attributes) = 0;
        virtual bool CreateEDPBuiltinEndpoints() = 0;
        /*
         * @description: 创建本地Writer的DiscoveredWriterData
         * @param {Writer} *writer
         * @param {TopicAttributes} &topicAttributes
         * @param {PublicationBuiltinTopicData} &writerQos
         * @return {*}
         */
        virtual bool CreateLocalDiscoveredWriterData(IWriter *writer, const TopicAttributes &topicAttributes, const PublicationBuiltinTopicData &writerQos);
        /*
         * @description: 创建本地Reader的DiscoveredReaderData
         * @param {Reader} *reader
         * @param {TopicAttributes} &topicAttributes
         * @param {SubscriptionBuiltinTopicData} &readerQos
         * @param {ContentFilterProperty_t} *content_filter
         * @return {*}
         */
        virtual bool CreateLocalDiscoveredReaderData(IReader *reader, const TopicAttributes &topicAttributes, const SubscriptionBuiltinTopicData &readerQos, const ContentFilterProperty_t *content_filter = nullptr);
        virtual bool ProcessLocalDiscoveredWriterData(IWriter *writer, DiscoveredWriterData *discoveredWriterData) = 0;
        virtual bool ProcessLocalDiscoveredReaderData(IReader *reader, DiscoveredReaderData *discoveredReaderData) = 0;
        /*
         * @description: 把本地的writer和远程发现的reader进行配对
         * @param {GUID} &guid
         * @param {DiscoveredReaderData} *discoveredReaderData
         * @return {*}
         */
        virtual bool PairLocalWriterAndRemoteReader(DiscoveredReaderData *discoveredReaderData);
        /*
         * @description: 把本地的reader和远程发现的writer进行配对
         * @param {GUID} &guid
         * @param {DiscoveredWriterData} *discoveredWriterData
         * @return {*}
         */
        virtual bool PairLocalReaderAndRemoteWriter(DiscoveredWriterData *discoveredWriterData);
        /*
         * @description: 取消和远程发现的reader配对
         * @param {GUID} &guid
         * @param {GUID} &readerGuid
         * @return {*}
         */
        virtual bool UnpairingRemoteReader(const GUID &parGuid, const GUID &readerGuid);
        /*
         * @description: 取消和远程发现的writer配对
         * @param {GUID} &guid
         * @param {GUID} &writerGuid
         * @return {*}
         */
        virtual bool UnpairingRemoteWriter(const GUID &parGuid, const GUID &writerGuid);
        virtual bool PairingWriter(IWriter *writer, DiscoveredWriterData *writerData);
        virtual bool PairingReader(IReader *reader, DiscoveredReaderData *readerData);
        virtual bool Matching(DiscoveredWriterData *writerData, DiscoveredReaderData *readerData);
        virtual bool PairLocalWriter(IWriter *writer, DiscoveredReaderData *readerData);
        virtual bool PairLocalReader(IReader *reader, DiscoveredWriterData *writerData);
        virtual void AssignRemoteEndpoints(DiscoveredParticipantData *parData) = 0;
        virtual bool RemoveLocalReader(IReader *reader) = 0;
        virtual bool RemoveLocalWriter(IWriter *writer) = 0;
        virtual bool RemoveRemoteEndpoint(std::shared_ptr<DiscoveredParticipantData> remoteParData) = 0;

        inline PDP *GetPDP()
        {
            return this->mPDP;
        }

    protected:
        PDP *mPDP;

        IParticipant *mParticipant;
    };
}

#endif