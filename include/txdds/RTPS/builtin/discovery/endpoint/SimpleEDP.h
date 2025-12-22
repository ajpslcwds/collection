/*
 * @Author       : songwenguang 563734@baosight.com
 * @Date         : 2024-07-19 17:43:17
 * @FilePath: /TXDDS/include/RTPS/builtin/discovery/endpoint/SimpleEDP.h
 * @Copyright (c) 2023 by BAOSIGHT, All Rights Reserved.
 * @Description  :
 */
#ifndef TXDDS_RTPS_SimpleEDP_H
#define TXDDS_RTPS_SimpleEDP_H

#include "txdds/RTPS/builtin/discovery/endpoint/EDP.h"
#include "txdds/RTPS/builtin/entity/BuiltinRTPSReader.h"
#include "txdds/RTPS/builtin/entity/BuiltinRTPSWriter.h"
#include "txdds/RTPS/builtin/discovery/endpoint/EDPListeners.h"

namespace BaoSky::rtps
{
    class SimpleEDPPubListener;
    class SimpleEDPSubListener;
    class SimpleEDP : public EDP
    {
    public:
        /*
         * @description: SimpleEDP类的构造函数
         * @param {PDP} *pdp
         * @param {IParticipant} *participantImpl
         * @return {*}
         */
        SimpleEDP(PDP *pdp, IParticipant *participant);
        virtual ~SimpleEDP();
        /*
         * @description: 初始化EDP配置
         * @param {BuiltinAttributes} &attributes
         * @return {*}
         */
        virtual bool InitEDP(const BuiltinAttributes &attributes) override;
        /*
         * @description: 创建EDP内置的writer和reader
         * @return {*}
         */
        virtual bool CreateEDPBuiltinEndpoints() override;
        /*
         * @description: 处理本地writer创建的DiscoveredWriterData
         * @param {Writer} *writer
         * @param {DiscoveredWriterData} *discoveredWriterData
         * @return {*}
         */
        virtual bool ProcessLocalDiscoveredWriterData(IWriter *writer, DiscoveredWriterData *discoveredWriterData) override;
        /*
         * @description: 处理本地reader创建的DiscoveredReaderData
         * @param {Reader} *reader
         * @param {DiscoveredReaderData} *discoveredReaderData
         * @return {*}
         */
        virtual bool ProcessLocalDiscoveredReaderData(IReader *reader, DiscoveredReaderData *discoveredReaderData) override;
        template <typename DiscoveredData>
        bool SerializeDiscoveredData(DiscoveredData *data, const BuiltinRTPSWriter<IStatefulWriter> &writer, CacheChange **change);
        void SetBuiltinReaderHistory(HistoryAttributes &history);
        void SetBuiltinWriterHistory(HistoryAttributes &history);
        void SetBuiltinReaderAttribute(ReaderAttributes &attr);
        void SetBuiltinWriterAttribute(WriterAttributes &attr);
        void SetEDPCommonAttribute(EndpointAttributes &endpointAttributes);

        virtual bool CreateEDPBuiltinWriter(IParticipant *participant, const std::string &topicName, const EntityId &entityId, const HistoryAttributes &historyAtt,
                                            WriterAttributes &wAtt, IWriterListener *listener, BuiltinRTPSWriter<IStatefulWriter> &edpWriter);
        virtual bool CreateEDPBuiltinReader(IParticipant *participant, const std::string &topicName, const EntityId &entityId, const HistoryAttributes &historyAtt,
                                            ReaderAttributes &rAtt, IReaderListener *listener, BuiltinRTPSReader<IStatefulReader> &edpReader);
        virtual void AssignRemoteEndpoints(DiscoveredParticipantData *parData) override;
        virtual bool RemoveLocalReader(IReader *reader) override;
        virtual bool RemoveLocalWriter(IWriter *writer) override;
        virtual bool RemoveRemoteEndpoint(std::shared_ptr<DiscoveredParticipantData> remoteParData) override;
        virtual void ClearEDPResource();

        BuiltinRTPSReader<IStatefulReader> mSEDPbuiltinSubscriptionsReader;
        BuiltinRTPSReader<IStatefulReader> mSEDPbuiltinPublicationsReader;
        BuiltinRTPSWriter<IStatefulWriter> mSEDPbuiltinPublicationsWriter;
        BuiltinRTPSWriter<IStatefulWriter> mSEDPbuiltinSubscriptionsWriter;

    private:
        BuiltinAttributes mBuiltinAttributes;
        SimpleEDPPubListener *mSimpleEDPPubListener;
        SimpleEDPSubListener *mSimpleEDPSubListener;
    };
}

#endif