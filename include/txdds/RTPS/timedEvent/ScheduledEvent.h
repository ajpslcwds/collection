/*
 * @Author       : songwenguang 563734@baosight.com
 * @Date         : 2024-08-01 11:39:05
 * @FilePath: /TXDDS/include/RTPS/timedEvent/ScheduledEvent.h
 * @Copyright (c) 2023 by BAOSIGHT, All Rights Reserved.
 * @Description  :
 */
#ifndef TXDDS_RTPS_TimedEvent_H
#define TXDDS_RTPS_TimedEvent_H
#include <chrono>
#include <atomic>
#include <functional>
using namespace std::chrono;

namespace BaoSky::rtps
{
    class TimedEventService;
    enum class EventStatus
    {
        ACTIVE,
        INVALID,
        WAITING
    };
    class ScheduledEvent
    {
    public:
        ScheduledEvent(TimedEventService &service, std::function<bool()> callbackFunc, uint64_t millisecondInterval);
        virtual ~ScheduledEvent();
        virtual void CancelScheduledEvent();
        virtual void StartScheduledEvent();
        virtual steady_clock::time_point GetNextExecuteTime();
        virtual bool CaculateEventExecuteTime(steady_clock::time_point now, steady_clock::time_point cancelTimepoint);
        virtual void ExecuteCallbackFunc(steady_clock::time_point now, steady_clock::time_point cancelTimepoint);
        virtual bool UpdateEventInterval(double interval);

    private:
        TimedEventService &mTimedEventService;
        std::function<bool()> mCallbackFunc;
        std::atomic<EventStatus> mEventStatus;
        std::atomic<microseconds> mEventExecuteInterval;
        std::atomic<steady_clock::time_point> mEventNextExecuteTime;
    };
}

#endif