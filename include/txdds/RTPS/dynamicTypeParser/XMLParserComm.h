/*
 * @Author       : baihaoran 948942547@qq.com
 * @Date         : 2025-05-16 16:25:33
 * @FilePath     : /TXDDS/include/RTPS/DynamicTypeBuilder/XMLParserComm.h
 * @Copyright (c) 2023 by BAOSIGHT, All Rights Reserved.
 * @Description  :
 */
#pragma once

namespace BaoSky::rtps::DynamicBuilder
{
    enum class XMLP_ret
    {
        XML_ERROR,
        XML_OK,
        XML_NOK
    };

    extern const char *DEFAULT_FASTRTPS_PROFILES;

    extern const char *ROOT;
    extern const char *PROFILES;
    extern const char *TYPES;
    extern const char *TYPE;
    extern const char *LOG;
    extern const char *LIBRARY_SETTINGS;
    extern const char *NAME;

    // TYPES parser
    extern const char *BOOLEAN;
    extern const char *CHAR;
    extern const char *WCHAR;
    extern const char *TBYTE;
    extern const char *OCTET;
    extern const char *UINT8;
    extern const char *INT8;
    extern const char *SHORT;
    extern const char *LONG;
    extern const char *USHORT;
    extern const char *ULONG;
    extern const char *LONGLONG;
    extern const char *ULONGLONG;
    extern const char *FLOAT;
    extern const char *DOUBLE;
    extern const char *LONGDOUBLE;
    extern const char *STRING;
    extern const char *WSTRING;
    extern const char *LITERAL;
    extern const char *STRUCT;
    extern const char *UNION;
    extern const char *SEQUENCE;
    extern const char *MAP;
    extern const char *TYPEDEF;
    extern const char *BITSET;
    extern const char *BITMASK;
    extern const char *ENUM;
    extern const char *CASE;
    extern const char *DEFAULT;
    extern const char *DISCRIMINATOR;
    extern const char *CASE_DISCRIMINATOR;
    extern const char *ARRAY_DIMENSIONS;
    extern const char *STR_MAXLENGTH;
    extern const char *SEQ_MAXLENGTH;
    extern const char *MAP_MAXLENGTH;
    extern const char *MAP_KEY_TYPE;
    extern const char *ENUMERATOR;
    extern const char *NON_BASIC_TYPE;
    extern const char *NON_BASIC_TYPE_NAME;
    extern const char *KEY;
    extern const char *MEMBER;
    extern const char *BITFIELD;
    extern const char *BIT_VALUE;
    extern const char *POSITION;
    extern const char *BIT_BOUND;
    extern const char *BASE_TYPE;

}
