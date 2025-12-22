#ifndef TXDDS_DCPS_INCOMPATIBLEQOSSTATUS_H
#define TXDDS_DCPS_INCOMPATIBLEQOSSTATUS_H

#include <txdds/DCPS/core/policy/QosPolicy.h>

#include <cstdint>
#include <vector>

namespace BaoSky::dds
{
    /**
     * @brief A struct storing the id of the incompatible QoS Policy and the number of times it fails
     */
    struct QosPolicyCount
    {
        //! Constructor
        QosPolicyCount() = default;

        //! Constructor
        QosPolicyCount(
            QosPolicyId_t id,
            int32_t c)
            : policy_id(id), count(c)
        {
        }

        //! @brief The id of the policy
        QosPolicyId_t policy_id = INVALID_QOS_POLICY_ID;

        //! @brief Total number of times that the concerned writer discovered a reader for the same topic
        //! @details The requested QoS is incompatible with the one offered by the writer
        uint32_t count = 0;
    };

    //! Alias of std::vector<QosPolicyCount>
    using QosPolicyCountSeq = std::vector<QosPolicyCount>;

    //! @brief A struct storing the requested incompatible QoS status
    struct IncompatibleQosStatus
    {
        //! @brief Total cumulative number of times the concerned writer discovered a reader for the same topic
        //! @details The requested QoS is incompatible with the one offered by the writer
        uint32_t total_count = 0;

        //! @brief The change in total_count since the last time the listener was called or the status was read
        uint32_t total_count_change = 0;

        //! @brief The id of the policy that was found to be incompatible the last time an incompatibility is detected
        QosPolicyId_t last_policy_id = INVALID_QOS_POLICY_ID;

        //! @brief A list of QosPolicyCount
        QosPolicyCountSeq policies;

        IncompatibleQosStatus()
            : policies(static_cast<size_t>(NEXT_QOS_POLICY_ID), QosPolicyCount(INVALID_QOS_POLICY_ID, 0))
        {
            for (uint32_t id = 0; id < NEXT_QOS_POLICY_ID; ++id)
            {
                policies[id].policy_id = static_cast<QosPolicyId_t>(id);
                policies[id].count = 0;
            }
        }
    };

    //! Alias of IncompatibleQosStatus
    using RequestedIncompatibleQosStatus = IncompatibleQosStatus;
    //! Alias of IncompatibleQosStatus
    using OfferedIncompatibleQosStatus = IncompatibleQosStatus;

}

#endif