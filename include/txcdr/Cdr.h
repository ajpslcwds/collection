/*
 * @Author       : yanli yanli563730@baosight.com
 * @Date         : 2025-02-17 13:01:27
 * @FilePath     : /txcdr/include/Cdr.h
 * @Copyright (c) 2025 by BAOSIGHT, All Rights Reserved.
 * @Description  :
 */

#ifndef TXCDR_CDR_H
#define TXCDR_CDR_H

#include <vector>
#include <string>
#include "txcdr/TXBuffer.h"
#include "txcdr/ReturnCode.h"
#include "txcdr/CdrSizeCalculator.h"

namespace BaoSky::Cdr
{
    const uint32_t MAX_SIZE = 100 * 1024 * 1024;
    class Cdr
    {
    public:
        Cdr(TXBuffer &cdr_buffer, const Endianness endianness = DEFAULT_ENDIAN, const CdrVersion cdr_version = XCDRv2);

        // Default endianess in the system.
        static const Endianness DEFAULT_ENDIAN;

        size_t GetSerializedDataLength() { return mOffsetIter - mCdrBuffer.begin(); };

        bool Resize(size_t min_size_inc);

        inline size_t Alignment(size_t data_size)
        {
            return data_size > mLastDataSize ? (data_size - ((mOffsetIter - mOriginIter) % data_size)) & (data_size - 1) : 0;
        }
        inline static size_t Alignment(
            size_t current_alignment,
            size_t data_size)
        {
            return (data_size - (current_alignment % data_size)) & (data_size - 1);
        }

        inline void MakeAlignment(size_t align)
        {
            mOffsetIter += align;
            mLastDataSize = 0;
        }
        void SetEncodingFlag(EncodingAlgorithmFlag encodingFlag)
        {
            mEncodingAlgorithmFlag = encodingFlag;
        }

        bool Jump(size_t numBytes);

        inline void ResetAlignment()
        {
            mOriginIter = mOffsetIter;
            mLastDataSize = 0;
        }

        inline Endianness endianness()
        {
            return (Endianness)mEndianness;
        }

        CdrVersion GetCdrVersion();

    protected:
        uint8_t mEndianness{Endianness::LITTLE_ENDIANNESS};

        CdrVersion mCdrVersion{CdrVersion::XCDRv2};

        EncodingAlgorithmFlag mEncodingAlgorithmFlag{EncodingAlgorithmFlag::PLAIN_CDR2};

        // The current position in the serialization/deserialization process.
        TXBufferIterator mOffsetIter;
        TXBufferIterator mPreviousOffsetIter;

        // The position from where the alignment is calculated.
        TXBufferIterator mOriginIter;

        // The last position in the buffer;
        TXBufferIterator mEndIter;

        // Reference to the buffer that will be serialized/deserialized.
        TXBuffer &mCdrBuffer;

        // This attribute specifies if it is needed to swap the bytes when the state is created.
        bool mSwapBytes{false};

        // Stores the last datasize serialized/deserialized when the state was created.
        size_t mLastDataSize{0};

        // Align for types equal or greater than 64bits.
        size_t mAlign64{4};

        // This attribute stores the option flags when the CDR type is DDS_CDR;
        std::array<uint8_t, 2> mOptions{{0}};
    };
}
#endif
