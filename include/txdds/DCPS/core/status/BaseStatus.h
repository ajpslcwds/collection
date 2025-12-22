#ifndef TXDDS_DCPS_BASESTATUS_H
#define TXDDS_DCPS_BASESTATUS_H

#include <cstdint>

namespace BaoSky::dds
{

    //! @brief A struct storing the base status
    struct BaseStatus
    {
        //! Total cumulative count
        int32_t total_count = 0;

        //! Increment since the last time the status was read
        int32_t total_count_change = 0;
    };

    //! Alias of BaseStatus
    using SampleLostStatus = BaseStatus;
    //! Alias of BaseStatus
    using LivelinessLostStatus = BaseStatus;
    //! Alias of BaseStatus
    using InconsistentTopicStatus = BaseStatus;

}
#endif
