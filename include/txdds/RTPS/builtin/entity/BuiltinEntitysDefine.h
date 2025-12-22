/*
 * @Author       : songwenguang 563734@baosight.com
 * @Date         : 2024-07-19 12:30:14
 * @FilePath: /TXDDS/include/RTPS/builtin/entity/BuiltinEntitysDefine.h
 * @Copyright (c) 2023 by BAOSIGHT, All Rights Reserved.
 * @Description  :
 */

#ifndef TXDDS_RTPS_BuiltinEntitysDefine_H
#define TXDDS_RTPS_BuiltinEntitysDefine_H
#include <cstdint>
#include <string>

#define DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER (0x00000001 << 0)
#define DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR (0x00000001 << 1)
#define DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER (0x00000001 << 2)
#define DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR (0x00000001 << 3)
#define DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER (0x00000001 << 4)
#define DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR (0x00000001 << 5)
#define BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER (0x00000001 << 10)
#define BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER (0x00000001 << 11)
#define DISC_BUILTIN_ENDPOINT_TOPICS_ANNOUNCER (0x00000001 << 28)
#define DISC_BUILTIN_ENDPOINT_TOPICS_DETECTOR (0x00000001 << 29)

namespace BaoSky::rtps
{
    static const std::string DCPS_PARTICIPANT = "DCPSParticipant";
    static const std::string DCPS_SUBSCRIPTION = "DCPSSubscription";
    static const std::string DCPS_PUBLICATION = "DCPSPublication";
    static const std::string DCPS_TOPIC = "DCPSTopic";

    struct BuiltinTopicKey_t
    {
        uint32_t mValue[3];
    };
}

#endif