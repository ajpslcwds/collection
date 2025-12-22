/*
 * @Author       : wengjiqing wengjiqing@baosight.com
 * @Date         : 2025-01-04 14:32:59
 * @FilePath     : /TXDDS/include/RTPS/history/CacheChange.h
 * Copyright (c) 2024 by BAOSIGHT, All Rights Reserved.
 * @Description  :
 */

#ifndef TXDDS_RTPS_CACHECHANGE_H
#define TXDDS_RTPS_CACHECHANGE_H

#include "txdds/RTPS/common/Guid.h"
#include "txdds/RTPS/common/SequenceNumber.h"
#include "txdds/RTPS/common/InstanceHandle.h"
#include "txdds/RTPS/common/SerializedPayload.h"
#include "txdds/RTPS/common/RTPSEntityTypes.h"
#include "txdds/RTPS/message/SubMessageElements.h"
#include "txdds/RTPS/history/IPayloadPool.h"

namespace BaoSky::rtps
{

    struct CacheChange
    {
        ChangeKind kind;

        GUID writerGuid;

        InstanceHandle instanceHandle;

        SequenceNumber sequenceNumber;

        SerializedPayload data_value;

        ParameterList inlineQos;

        Time sourceTimestamp;

        bool hasBeenRead = false;

        bool canBeRead;

        bool isFragmentData;

        FragmentNumberSet_t missingData;

        uint16_t mMaxFragmentSize = 65000;

        uint32_t GetFragments()
        {
            return (data_value.length / mMaxFragmentSize) + ((data_value.length % mMaxFragmentSize) ? 1 : 0);
        }

        IPayloadPool *mPayloadOwner = nullptr;
        CacheChange()
        {
            writerGuid = GUID_UNKNOWN;
            kind = ChangeKind::ALIVE;
            canBeRead = false;
            hasBeenRead = false;
            isFragmentData = false;
        }
        bool Copy(const CacheChange *ch_ptr)
        {
            kind = ch_ptr->kind;
            writerGuid = ch_ptr->writerGuid;
            instanceHandle = ch_ptr->instanceHandle;
            sequenceNumber = ch_ptr->sequenceNumber;
            sourceTimestamp = ch_ptr->sourceTimestamp;
            canBeRead = ch_ptr->canBeRead;
            hasBeenRead = ch_ptr->hasBeenRead;
            missingData = ch_ptr->missingData;
            mMaxFragmentSize = ch_ptr->mMaxFragmentSize;
            isFragmentData = ch_ptr->isFragmentData;
            inlineQos = ch_ptr->inlineQos;
            return data_value.copy(&ch_ptr->data_value, false);
        }

        void copy_not_memcpy(const CacheChange *ch_ptr)
        {
            kind = ch_ptr->kind;
            writerGuid = ch_ptr->writerGuid;
            instanceHandle = ch_ptr->instanceHandle;
            sequenceNumber = ch_ptr->sequenceNumber;
            sourceTimestamp = ch_ptr->sourceTimestamp;
            canBeRead = ch_ptr->canBeRead;
            hasBeenRead = ch_ptr->hasBeenRead;
            missingData = ch_ptr->missingData;
            mMaxFragmentSize = ch_ptr->mMaxFragmentSize;
            isFragmentData = ch_ptr->isFragmentData;
            inlineQos = ch_ptr->inlineQos;

            // Copy certain values from serializedPayload
            // data_value.encapsulation = ch_ptr->data_value.encapsulation;

            // Copy fragment size and calculate fragment count
            // setFragmentSize(ch_ptr->fragment_size_, false);
        }
    };
}

#endif