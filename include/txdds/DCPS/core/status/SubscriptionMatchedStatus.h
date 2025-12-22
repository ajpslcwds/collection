#ifndef _SUBSCRIPTION_MATCHED_STATUS_HPP_
#define _SUBSCRIPTION_MATCHED_STATUS_HPP_

#include <cstdint>

#include "txdds/DCPS/core/status/MatchedStatus.h"
#include "txdds/DCPS/common/InstanceHandle.h"

namespace BaoSky::dds
{
    //! @brief A structure storing the subscription status
    struct SubscriptionMatchedStatus : public MatchedStatus
    {
        //! @brief Handle to the last writer that matched the reader causing the status change
        InstanceHandle last_publication_handle;
    };

}

#endif //_SUBCRIPTION_MATCHED_STATUS_HPP_
