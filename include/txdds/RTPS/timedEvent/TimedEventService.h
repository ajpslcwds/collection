/*
 * @Author       : songwenguang 563734@baosight.com
 * @Date         : 2024-08-01 11:39:05
 * @FilePath: /TXDDS/include/RTPS/timedEvent/TimedEventService.h
 * @Copyright (c) 2023 by BAOSIGHT, All Rights Reserved.
 * @Description  :
 */
#ifndef TXDDS_RTPS_TimedEventService_H
#define TXDDS_RTPS_TimedEventService_H

#include "txdds/RTPS/timedEvent/ScheduledEvent.h"
#include "txdds/RTPS/timedEvent/ConditionVariable.h"
#include "txdds/RTPS/transport/ThreadConfig.h"

#include <mutex>
#include <cstdint>
#include <vector>
#include <thread>

namespace BaoSky::rtps
{
    class ScheduledEvent;
    class TimedEventService
    {
    public:
        TimedEventService();
        ~TimedEventService();
        void CreateThread(const ThreadConfig &config);
        void StopThread();
        bool RegisterTimedEvent(ScheduledEvent *event);
        void UnregisterTimedEvent(ScheduledEvent *event);
        void NotifyEvent(ScheduledEvent *event);
        void NotifyEvent(ScheduledEvent *event, const steady_clock::time_point &cancelTime);
        std::thread mThread;

    private:
        std::atomic<bool> mIsStopThread;
        std::timed_mutex mMtx;
        ConditionVariable mEventCV;
        uint32_t mTimedEventCount;
        std::vector<ScheduledEvent *> mWaitTimedEvent;
        std::vector<ScheduledEvent *> mActiveTimedEvent;
        std::atomic<bool> mIsCheckActiveTimer;
        std::chrono::steady_clock::time_point mCurrentTime;
        bool mAllowVecManipulation;
        ConditionVariable mVecManipulationCV;

        bool RegisterTimer(ScheduledEvent *event);
        void TriggerTimerEvent();
        void ExecuteTimedEvent();
    };
}

#endif