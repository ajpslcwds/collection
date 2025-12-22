/*
 * @Author: songwenguang 563734@baosight.com
 * @Date: 2023-05-20 15:26:54
 * @FilePath: /TXDDS/include/RTPS/timedEvent/ConditionVariable.h
 * @Copyright (c) 2025 by BAOSIGHT, All Rights Reserved.
 * @Description:
 */

#ifndef TXDDS_RTPS_ConditionVariable_H
#define TXDDS_RTPS_ConditionVariable_H
#include <pthread.h>
#include <mutex>
#include <chrono>

using namespace std::chrono;
namespace BaoSky::rtps
{
    class ConditionVariable
    {
    public:
        ConditionVariable()
        {
            pthread_condattr_init(&mCondAttr);
            pthread_condattr_setclock(&mCondAttr, CLOCK_MONOTONIC);
            pthread_cond_init(&mCond, &mCondAttr);
        }

        void WaitUntil(std::unique_lock<std::timed_mutex> &lock, const steady_clock::time_point &blockTimePoint)
        {
            auto sec = time_point_cast<seconds>(blockTimePoint);
            auto nanoSec = time_point_cast<nanoseconds>(blockTimePoint) - time_point_cast<nanoseconds>(sec);
            struct timespec maxBlockTime = {sec.time_since_epoch().count(), nanoSec.count()};
            pthread_cond_timedwait(&mCond, lock.mutex()->native_handle(), &maxBlockTime);
        }

        void Wait(std::unique_lock<std::timed_mutex> &lock, std::function<bool()> predicate)
        {
            while (!predicate())
            {
                pthread_cond_wait(&mCond, lock.mutex()->native_handle());
            }
        }

        void NotifyOne()
        {
            pthread_cond_signal(&mCond);
        }

        void NotifyAll()
        {
            pthread_cond_broadcast(&mCond);
        }

    private:
        pthread_condattr_t mCondAttr;
        pthread_cond_t mCond;
    };
}
#endif