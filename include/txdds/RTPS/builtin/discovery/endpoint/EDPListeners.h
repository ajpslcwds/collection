/*
 * @Author       : songwenguang 563734@baosight.com
 * @Date         : 2024-07-19 17:51:21
 * @FilePath: /TXDDS/include/RTPS/builtin/discovery/endpoint/EDPListeners.h
 * @Copyright (c) 2023 by BAOSIGHT, All Rights Reserved.
 * @Description  :
 */

#ifndef TXDDS_RTPS_EDPListeners_H
#define TXDDS_RTPS_EDPListeners_H
#include "txdds/RTPS/builtin/discovery/endpoint/SimpleEDP.h"
#include "txdds/RTPS/reader/IReaderListener.h"
#include "txdds/RTPS/writer/IWriterListener.h"
#include "txdds/RTPS/history/CacheChange.h"

namespace BaoSky::rtps
{
    class SimpleEDP;

    class EDPPubListener : public IWriterListener, public IReaderListener
    {
    public:
        EDPPubListener() = default;
        virtual ~EDPPubListener() override;

    protected:
        virtual void AddWriterFromCacheChange(IReader *reader, IReaderHistory *readerHistory, CacheChange *change,
                                              EDP *edp, bool releaseChange = true);
    };

    class EDPSubListener : public IWriterListener, public IReaderListener
    {
    public:
        EDPSubListener() = default;
        virtual ~EDPSubListener() override;

    protected:
        virtual void AddReaderFromCacheChange(IReader *reader, IReaderHistory *readerHistory, CacheChange *change,
                                              EDP *edp, bool releaseChange = true);
    };

    class SimpleEDPPubListener : public EDPPubListener
    {
    public:
        SimpleEDPPubListener(SimpleEDP *simpleEDP)
            : mSimpleEDP(simpleEDP)
        {
        }
        virtual ~SimpleEDPPubListener() override;
        virtual void OnNewCacheChangeAdded(IReader *reader, const CacheChange *const change) override;
        virtual void OnWriterChangeReceivedByAll(IWriter *writer, CacheChange *change);

    protected:
        SimpleEDP *mSimpleEDP;
    };

    class SimpleEDPSubListener : public EDPSubListener
    {
    public:
        SimpleEDPSubListener(SimpleEDP *simpleEDP)
            : mSimpleEDP(simpleEDP)
        {
        }
        virtual ~SimpleEDPSubListener() override;
        virtual void OnNewCacheChangeAdded(IReader *reader, const CacheChange *const change) override;
        virtual void OnWriterChangeReceivedByAll(IWriter *writer, CacheChange *change);

    private:
        SimpleEDP *mSimpleEDP;
    };
}

#endif