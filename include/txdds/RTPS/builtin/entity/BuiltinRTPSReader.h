/*
 * @Author       : songwenguang 563734@baosight.com
 * @Date         : 2024-07-19 17:54:39
 * @FilePath: /TXDDS/include/RTPS/builtin/entity/BuiltinRTPSReader.h
 * @Copyright (c) 2023 by BAOSIGHT, All Rights Reserved.
 * @Description  :
 */
#ifndef TXDDS_RTPS_BuiltinRTPSReader_H
#define TXDDS_RTPS_BuiltinRTPSReader_H

#include "txdds/RTPS/history/ReaderHistory.h"
#include "txdds/RTPS/reader/IReaderListener.h"

#include <memory>

namespace BaoSky::rtps
{
    template <typename T>
    class BuiltinRTPSReader
    {
    public:
        BuiltinRTPSReader()
            : mBuiltinReader(nullptr), mReaderHistory(nullptr), mReaderListener(nullptr)
        {
        }

        virtual ~BuiltinRTPSReader()
        {
        }

        T *mBuiltinReader;
        std::unique_ptr<IReaderHistory> mReaderHistory;
        std::unique_ptr<IReaderListener> mReaderListener;
    };
}

#endif