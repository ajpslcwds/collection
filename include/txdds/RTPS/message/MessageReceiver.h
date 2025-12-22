/*
 * @Author      : wengjiqing wengjiqing@baosight.com
 * @Date        : 2024-07-31 15:22:09
 * @FilePath: /TXDDS/include/RTPS/message/MessageReceiver.h
 * Copyright (c) 2024 by BAOSIGHT, All Rights Reserved.
 * @Description :
 */
#ifndef TXDDS_RTPS_MESSAGERECEIVE_H
#define TXDDS_RTPS_MESSAGERECEIVE_H

#include <txdds/RTPS/common/RTPSMessageTypes.h>
#include <txdds/RTPS/common/RTPSEntityTypes.h>
#include <txdds/RTPS/common/GuidPrefix.h>
#include <txdds/RTPS/common/LocatorList_t.h>
#include <txdds/RTPS/common/Parameter.h>
#include <txdds/RTPS/common/CDRMessage.h>
#include <txdds/RTPS/common/Time.h>
#include <txdds/RTPS/message/SubMessageHeader.h>
#include <txdds/RTPS/message/submessages/Data.h>
#include <txdds/RTPS/message/submessages/HeartBeat.h>
#include <txdds/RTPS/message/submessages/AckNack.h>
#include <txdds/RTPS/message/submessages/Gap.h>
#include <txdds/RTPS/message/submessages/InfoDestination.h>
#include <txdds/RTPS/message/submessages/InfoTimestamp.h>
#include <txdds/RTPS/message/submessages/DataFrag.h>
#include <txdds/RTPS/message/submessages/HeartBeatFrag.h>
#include <txdds/RTPS/message/submessages/NackFrag.h>
#include <map>
#include <mutex>
#include <condition_variable>
namespace BaoSky::rtps
{
    class IParticipant;
    class IEndpoint;
    class CacheChange;

    struct MessageReceiverData
    {
        // send sourceVersion
        ProtocolVersion_t sourceVersion;
        // send sourceVendorId
        VendorId_t sourceVendorId;
        // send GuidPrefix
        GuidPrefix sourceGuidPrefix;
        // receive GuidPrefix
        GuidPrefix destGuidPrefix;
        LocatorList_t unicastReplyLocatorList;
        LocatorList_t multicastReplyLocatorList;
        bool haveTimestamp;
        // change if has HeaderExtension
        Time timestamp;
        // change if has HeaderExtension
        MessageLength_t messageLength;
        // change if has HeaderExtension
        Checksum_t messageChecksum;
        // change if has HeaderExtension
        Time rtpsSendTimestamp;
        Time rtpsReceptionTimestamp;
        bool clockSkewDetected;
        // change if has HeaderExtension
        std::vector<Parameter> parameters;
    };

    class MessageReceiver
    {
    protected:
        MessageReceiverData mData;
        CDRMessage *mCdrMsg;
        IParticipant *mParticipant;
        std::map<EntityId, IEndpoint *> mReaders;
        std::map<EntityId, IEndpoint *> mWriters;
        GuidPrefix mPrefix;
        bool mOpFlag;
        std::mutex mOpFlagMtx;
        std::condition_variable mOpFlagCV;

        /**
         * @Description : 根据 mCdrMsg 解析 RTPS 头，并将 guid 前缀、协议版本、供应商等信息存入 message receive
         * @return       {bool} 解析是否成功
         */
        virtual bool processRTPSHeader();

        /**
         * @Description : 根据 mCdrMsg 解析 SubmessageHeader，包含submessageId、flag、messageLength等信息
         * @param        header：output param
         * @return       {bool} 解析是否成功
         */
        virtual bool processSubMessageHeader(SubmessageHeader &header);

        /**
         * @Description : 根据 mCdrMsg 以及 SubmessageHeader 解析 DataMsg，其中包含readerId、writerId、payload等信息
         * @param        header: input param
         * @param        data: output param
         * @return       {bool} 解析是否成功
         */
        virtual bool processDataMsg(SubmessageHeader header, DataMsg &data, CacheChange *ch);

        /**
         * @Description : 根据 mCdrMsg 以及 SubmessageHeader 解析 HeartBeatMsg
         * @param        header: input param
         * @param        data: output param
         * @return       {bool} 解析是否成功
         */
        virtual bool processHeartBeatMsg(SubmessageHeader header, HeartBeatMsg &data);

        /**
         * @Description : 根据 mCdrMsg 以及 SubmessageHeader 解析 AckNackMsg
         * @param        header: input param
         * @param        data: output param
         * @return       {bool} 解析是否成功
         */
        virtual bool processAckNackMsg(SubmessageHeader header, AckNackMsg &data);

        /**
         * @Description : 根据 mCdrMsg 以及 SubmessageHeader 解析 GapMsg
         * @param        header: input param
         * @param        data: output param
         * @return       {bool} 解析是否成功
         */
        virtual bool processGapMsg(SubmessageHeader header, GapMsg &data);
        virtual bool processInfoDST(SubmessageHeader header, InfoDestinationMsg &data);
        virtual bool processInfoTimestamp(SubmessageHeader header, InfoTimestampMsg &data);
        virtual bool processNackFragMsg(SubmessageHeader header, NackFragMsg &data);
        virtual bool processHeartBeatFragMsg(SubmessageHeader header, HeartBeatFragMsg &data);
        virtual bool processDataFragMsg(SubmessageHeader header, DataFragMsg &data, CacheChange *ch);
        virtual bool processDataMsgInlineQos(BaoSky::rtps::CDRMessage &cdrMsg, CacheChange *ch);
        virtual void NotifyData(EntityId id, CacheChange *ch);
        virtual void NotifyGap(GapMsg gapMsg);
        virtual void NotifyHeartBeat(HeartBeatMsg heartBeatMsg);
        virtual void NotifyAckNack(AckNackMsg ackMsg);
        virtual void NotifyDataFrag(CacheChange *ch, DataFragMsg datafrag);
        virtual void NotifyHeartBeatFrag(HeartBeatFragMsg heartbeat);
        virtual void NotifyNackFrag(NackFragMsg nack);

    public:
        virtual uint32_t Count()
        {
            return mReaders.size() + mWriters.size();
        }
        MessageReceiver(IParticipant *participant = nullptr);
        virtual void onDataReceive(CDRMessage *cdrMsg);
        virtual void addEndPoint(IEndpoint *endpoint);
        virtual void removeEndPoint(IEndpoint *endpoint);
        virtual ~MessageReceiver();
        std::mutex mMtx;
        std::map<GuidPrefix, uint32_t> mMapPrefix;
    };
}

#endif
