/*
 * @Author       : songwenguang 563734@baosight.com
 * @Date         : 2024-07-19 17:49:28
 * @FilePath: /TXDDS/include/RTPS/builtin/BuiltinProtocol.h
 * @Copyright (c) 2023 by BAOSIGHT, All Rights Reserved.
 * @Description  :
 */
#ifndef TXDDS_RTPS_BuiltinProtocol_H
#define TXDDS_RTPS_BuiltinProtocol_H

#include "txdds/RTPS/builtin/discovery/participant/PDP.h"
#include "txdds/RTPS/common/LocatorList_t.h"
#include "txdds/RTPS/participant/IParticipant.h"
#include "txdds/RTPS/attributes/BuiltinAttributes.h"
#include "txdds/RTPS/reader/IReader.h"
#include "txdds/DCPS/builtin/topic/PublicationBuiltinTopicData.h"
#include "txdds/DCPS/builtin/topic/SubscriptionBuiltinTopicData.h"
#include "txdds/RTPS/attributes/TopicAttributes.h"

using PublicationBuiltinTopicData = BaoSky::dds::builtin::PublicationBuiltinTopicData;
using SubscriptionBuiltinTopicData = BaoSky::dds::builtin::SubscriptionBuiltinTopicData;

namespace BaoSky::rtps
{
    class TopicAttributes;
    class PDP;

    class BuiltinProtocol
    {
    public:
        BuiltinProtocol();
        virtual ~BuiltinProtocol();
        bool InitBuiltinProtocol(IParticipant *participant, BuiltinAttributes builtinAttributes);
        bool EnablePDP();
        bool AddLocalWriter(IWriter *writer, const TopicAttributes &topicAttributes, const PublicationBuiltinTopicData &writerQos);
        bool AddLocalReader(IReader *reader, const TopicAttributes &topicAttributes, const SubscriptionBuiltinTopicData &readerQos, const ContentFilterProperty_t *filter = nullptr);
        bool RemoveLocalWriter(IWriter *writer);
        bool RemoveLocalReader(IReader *reader);
        virtual bool StopAnnouncement();
        virtual bool ResetAnnouncement();
        virtual bool EnableDiscovery();
        virtual PDP *GetPDPPtr();
        IParticipant *mParticipant;
        BuiltinAttributes mBuiltinAttributes;
        PDP *mPDP;
    };
}

#endif