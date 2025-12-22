/*
 * @Author       : wengjiqing wengjiqing@baosight.com
 * @Date         : 2025-07-01 13:38:51
 * @FilePath     : /TXDDS/include/txdds/RTPS/common/PortParameters.h
 * Copyright (c) 2024 by BAOSIGHT, All Rights Reserved.
 * @Description  : 
 */
#ifndef TXDDS_RTPS_PORTPARAMETERS_H
#define TXDDS_RTPS_PORTPARAMETERS_H

#include <cstdint>

namespace BaoSky::rtps
{
    class PortParameters
    {
    public:
        PortParameters() : mPortBase(7400),
                           mDomainIDGain(250),
                           mParticipantIDGain(2),
                           mOffsetd0(0),
                           mOffsetd1(10),
                           mOffsetd2(1),
                           mOffsetd3(11)
        {
        }

        virtual ~PortParameters()
        {
        }

        bool operator==(const PortParameters &b) const
        {
            return (this->mPortBase == b.mPortBase) &&
                   (this->mDomainIDGain == b.mDomainIDGain) &&
                   (this->mParticipantIDGain == b.mParticipantIDGain) &&
                   (this->mOffsetd0 == b.mOffsetd0) &&
                   (this->mOffsetd1 == b.mOffsetd1) &&
                   (this->mOffsetd2 == b.mOffsetd2) &&
                   (this->mOffsetd3 == b.mOffsetd3);
        }

        inline uint32_t GetMulticastPort(uint32_t domainId) const
        {
            uint32_t port = mPortBase + mDomainIDGain * domainId + mOffsetd0;

            if (port > 65535)
            {
                return 0;
            }

            return port;
        }

        inline uint32_t GetUnicastPort(uint32_t domainId, uint32_t RTPSParticipantID) const
        {
            uint32_t port = mPortBase + mDomainIDGain * domainId + mOffsetd1 + mParticipantIDGain * RTPSParticipantID;

            if (port > 65535)
            {
                return 0;
            }

            return port;
        }

        inline uint16_t CalculateWellKnownPort(uint32_t domain_id, uint32_t RTPSParticipantID, bool is_multicast = false) const
        {
            uint32_t port = mPortBase + mDomainIDGain * domain_id + (is_multicast ? mOffsetd2 : mOffsetd3 + mParticipantIDGain * RTPSParticipantID);
            if (port > 65535)
            {
                exit(EXIT_FAILURE);
            }
            return static_cast<uint16_t>(port);
        }

    public:
        uint16_t mPortBase;
        uint16_t mDomainIDGain;
        uint16_t mParticipantIDGain;
        uint16_t mOffsetd0;
        uint16_t mOffsetd1;
        uint16_t mOffsetd2;
        uint16_t mOffsetd3;
    };
}

#endif