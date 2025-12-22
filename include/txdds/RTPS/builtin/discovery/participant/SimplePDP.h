/*
 * @Author       : songwenguang 563734@baosight.com
 * @Date         : 2024-07-19 17:43:53
 * @FilePath: /TXDDS/include/RTPS/builtin/discovery/participant/SimplePDP.h
 * @Copyright (c) 2023 by BAOSIGHT, All Rights Reserved.
 * @Description  :
 */
#ifndef TXDDS_RTPS_SimplePDP_H
#define TXDDS_RTPS_SimplePDP_H

#include "txdds/RTPS/builtin/discovery/participant/PDP.h"
#include "txdds/RTPS/builtin/entity/BuiltinRTPSReader.h"
#include "txdds/RTPS/builtin/entity/BuiltinRTPSWriter.h"
#include "txdds/RTPS/attributes/HistoryAttributes.h"
#include "txdds/RTPS/attributes/WriterAttributes.h"
#include "txdds/RTPS/attributes/ReaderAttributes.h"

namespace BaoSky::rtps
{
    class IStatelessReader;
    class IStatelessWriter;

    class SimplePDP : public PDP
    {
    public:
        SimplePDP(BuiltinProtocol *builtinProtocol);
        virtual ~SimplePDP();
        /*
         * @description: 根据participant的配置初始化
         * @param {IParticipant} *participant
         * @return {*}
         */
        virtual bool Init(IParticipant *participant) override;
        /*
         * @description: 创建spdp builtin writer及reader
         * @return {*}
         */
        virtual bool CreatePDPBuiltinEndpoints() override;
        /*
         * @description: 向外公告participant状态
         * @param {bool} isNewChange
         * @param {bool} isDispose
         * @return {*}
         */
        virtual void AnnounceParticipantState(bool isNewChange, bool isDispose = false) override;
        /*
         * @description: 根据探测到的DiscoveredParticipantData信息赋值远程端点
         * @param {DiscoveredParticipantData} *discoveredParticipantData
         * @return {*}
         */
        virtual void AssignRemoteEndpoints(DiscoveredParticipantData *discoveredParticipantData) override;
        /*
         * @description: 添加匹配的远程端点
         * @param {DiscoveredParticipantData} *discoveredParticipantData
         * @return {*}
         */
        void MatchRemoteEndpoints(DiscoveredParticipantData *discoveredParticipantData);
        /*
         * @description: 设置writerhistory属性
         * @return {*}
         */
        HistoryAttributes SetWriterHistoryAttributes();
        /*
         * @description: 设置readerhistory属性
         * @return {*}
         */
        HistoryAttributes SetReaderHistoryAttributes();
        /*
         * @description: 初始化spdp本地participant的DiscoveredParticipantData
         * @param {DiscoveredParticipantData} *discoveredParticipantData
         * @return {*}
         */
        virtual bool InitDiscoveredParticipantData(std::shared_ptr<DiscoveredParticipantData> discoveredParticipantData);
        /*
         * @description: 创建DiscoveredParticipantData
         * @param {DiscoveredParticipantData} *discoveredParticipantData
         * @param {GUID} &writerGuid
         * @return {*}
         */
        virtual std::shared_ptr<DiscoveredParticipantData> CreateParticipantProxyData(DiscoveredParticipantData *remoteDiscoveredParticipantData, const GUID &writerGuid) override;
        virtual bool RemoveRemoteEndpoint(const DiscoveredParticipantData &remoteParData) override;
        void ClearSPDPResource();
        virtual bool CreateSPDPWriter();
        virtual bool CreateSPDPReader();

    private:
        BuiltinRTPSReader<IStatelessReader> mSPDPbuiltinParticipantReader;

        BuiltinRTPSWriter<IStatelessWriter> mSPDPbuiltinParticipantWriter;
    };
}

#endif