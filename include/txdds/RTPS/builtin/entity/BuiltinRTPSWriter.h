/*
 * @Author       : songwenguang 563734@baosight.com
 * @Date         : 2024-07-19 17:55:06
 * @FilePath: /TXDDS/include/RTPS/builtin/entity/BuiltinRTPSWriter.h
 * @Copyright (c) 2023 by BAOSIGHT, All Rights Reserved.
 * @Description  :
 */
#ifndef TXDDS_RTPS_BuiltinRTPSWriter_H
#define TXDDS_RTPS_BuiltinRTPSWriter_H

#include "txdds/RTPS/history/IWriterHistory.h"

#include <memory>

namespace BaoSky::rtps
{
    template <typename T>
    class BuiltinRTPSWriter
    {
    public:
        BuiltinRTPSWriter()
            : mBuiltinWriter(nullptr), mWriterHistory(nullptr)
        {
        }
        virtual ~BuiltinRTPSWriter()
        {
        }

        T *mBuiltinWriter;
        std::unique_ptr<IWriterHistory> mWriterHistory;
    };
}

#endif