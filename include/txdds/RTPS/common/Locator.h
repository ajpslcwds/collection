/*
 * @Author       : songwenguang 563734@baosight.com
 * @Date         : 2024-07-16 14:26:17
 * @FilePath: /TXDDS/include/RTPS/common/Locator.h
 * @Copyright (c) 2023 by BAOSIGHT, All Rights Reserved.
 * @Description  :
 */
#ifndef TXDDS_RTPS_Locator_H
#define TXDDS_RTPS_Locator_H

#include "txdds/RTPS/utils/IPLocator.h"
#include <sstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <memory>
namespace BaoSky::rtps
{
    using octet = unsigned char;
#define LOCATOR_INVALID(loc)                   \
    {                                          \
        loc.mKind = LOCATOR_KIND_INVALID;      \
        loc.mPort = LOCATOR_PORT_INVALID;      \
        LOCATOR_ADDRESS_INVALID(loc.mAddress); \
    }
#define LOCATOR_KIND_INVALID -1
#define LOCATOR_ADDRESS_INVALID(a)           \
    {                                        \
        memset(a, 0x00, 16 * sizeof(octet)); \
    }
#define LOCATOR_PORT_INVALID 0
#define LOCATOR_KIND_RESERVED 0
#define LOCATOR_KIND_UDPv4 1
#define LOCATOR_KIND_UDPv6 2
#define LOCATOR_KIND_TCPv4 4
#define LOCATOR_KIND_TCPv6 8
#define LOCATOR_KIND_SHM 16
    class Locator
    {
    public:
        Locator() : mKind(LOCATOR_KIND_UDPv4), mPort(0)
        {
            LOCATOR_ADDRESS_INVALID(mAddress);
        }
        Locator(
            uint32_t portin)
            : mKind(LOCATOR_KIND_UDPv4)
        {
            mPort = portin;
            LOCATOR_ADDRESS_INVALID(mAddress);
        }
        Locator(int32_t kind, uint32_t port) : mKind(kind), mPort(port)
        {
            LOCATOR_ADDRESS_INVALID(mAddress);
        }
        void SetAddress(const Locator &locator)
        {
            memcpy(mAddress, locator.mAddress, sizeof(mAddress));
        }
        unsigned char *GetAddress()
        {
            return mAddress;
        }
        void DisableAddress()
        {
            LOCATOR_ADDRESS_INVALID(mAddress);
        }
        int32_t mKind;
        uint32_t mPort;
        octet mAddress[16];
        std::string mConfigName;
    };

    inline bool IsAddressDefined(const Locator &loc)
    {
        if (loc.mKind == LOCATOR_KIND_UDPv4 || loc.mKind == LOCATOR_KIND_TCPv4)
        {
            for (uint8_t i = 12; i < 16; ++i)
            {
                if (loc.mAddress[i] != 0)
                {
                    return true;
                }
            }
        }
        else if (loc.mKind == LOCATOR_KIND_UDPv6 || loc.mKind == LOCATOR_KIND_TCPv6)
        {
            for (uint8_t i = 0; i < 16; ++i)
            {
                if (loc.mAddress[i] != 0)
                {
                    return true;
                }
            }
        }
        return false;
    }

    inline bool IsLocatorValid(const Locator &loc)
    {
        return (0 <= loc.mKind);
    }


    inline bool operator<(const Locator &loc1, const Locator &loc2)
    {
        return memcmp(&loc1, &loc2, sizeof(Locator)) < 0;
    }

    inline bool operator==(const Locator &loc1, const Locator &loc2)
    {
        if (loc1.mKind != loc2.mKind)
        {
            return false;
        }
        if (loc1.mPort != loc2.mPort)
        {
            return false;
        }
        if (!std::equal(loc1.mAddress, loc1.mAddress + 16, loc2.mAddress))
        {
            return false;
        }
        return true;
    }

    inline bool operator!=(const Locator &loc1, const Locator &loc2)
    {
        return !(loc1 == loc2);
    }


    inline std::ostream &operator<<(std::ostream &output, const Locator &loc)
    {
        switch (loc.mKind)
        {
        case LOCATOR_KIND_TCPv4:
        {
            output << "TCPv4:[";
            break;
        }
        case LOCATOR_KIND_UDPv4:
        {
            output << "UDPv4:[";
            break;
        }
        case LOCATOR_KIND_TCPv6:
        {
            output << "TCPv6:[";
            break;
        }
        case LOCATOR_KIND_UDPv6:
        {
            output << "UDPv6:[";
            break;
        }
        case LOCATOR_KIND_SHM:
        {
            output << "SHM:[";
            break;
        }
        default:
        {
            output << "Invalid_locator:[_]:0";
            return output;
        }
        }
        if (loc.mKind == LOCATOR_KIND_UDPv4 || loc.mKind == LOCATOR_KIND_TCPv4)
        {
            output << IPLocator::toIPv4string(loc);
        }
        else if (loc.mKind == LOCATOR_KIND_UDPv6 || loc.mKind == LOCATOR_KIND_TCPv6)
        {
            output << IPLocator::toIPv6string(loc);
        }
        else if (loc.mKind == LOCATOR_KIND_SHM)
        {
            if (loc.mAddress[0] == 'M')
            {
                output << "M";
            }
            else
            {
                output << "_";
            }
        }
        if (loc.mKind == LOCATOR_KIND_TCPv4 || loc.mKind == LOCATOR_KIND_TCPv6)
        {
            output << "]:" << std::to_string(IPLocator::getPhysicalPort(loc)) << "-" << std::to_string(IPLocator::getLogicalPort(loc));
        }
        else
        {
            output << "]:" << loc.mPort;
        }

        return output;
    }
    typedef std::vector<Locator>::iterator LocatorListIterator;
    typedef std::vector<Locator>::const_iterator LocatorListConstIterator;
}

#endif