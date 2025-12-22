/*
 * @Author       : songwenguang 563734@baosight.com
 * @Date         : 2024-07-11 16:24:30
 * @FilePath: /TXDDS/include/RTPS/common/RTPSModelTypes.h
 * @Copyright (c) 2023 by BAOSIGHT, All Rights Reserved.
 * @Description  :
 */
namespace BaoSky::rtps
{
    // ChangeForReaderStatusKind
    enum class ChangeForReaderStatusKind
    {
        UNSENT,
        UNACKNOWLEDGED,
        REQUESTED,
        ACKNOWLEDGED,
        UNDERWAY
    };

    // ChangeFromWriterStatusKind
    enum class ChangeFromWriterStatusKind
    {
        NA_FILTERED,
        NA_REMOVED,
        NA_UNSPECIFIED,
        MISSING,
        RECEIVED,
        UNKNOWN
    };
    // todo:ParticipantMessageData
}
