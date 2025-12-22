/*
 * @Author: songwenguang 563734@baosight.com
 * @Date: 2025-03-28 16:50:41
 * @FilePath: /TXDDS/include/DCPS/dynamicType/CommonType.h
 * @Copyright (c) 2025 by BAOSIGHT, All Rights Reserved.
 * @Description:
 */

#pragma once
#include <cstdint>
#include <string>
namespace BaoSky::dds
{
    typedef uint32_t MemberId;
    const MemberId MEMBER_ID_INVALID = 0x0fffffff;

    typedef unsigned char TypeKind;
    using octet = unsigned char;
    const octet TK_NONE = 0x00;
    const octet TK_BOOLEAN = 0x01;
    const octet TK_BYTE = 0x02;
    const octet TK_INT16 = 0x03;
    const octet TK_INT32 = 0x04;
    const octet TK_INT64 = 0x05;
    const octet TK_UINT16 = 0x06;
    const octet TK_UINT32 = 0x07;
    const octet TK_UINT64 = 0x08;
    const octet TK_FLOAT32 = 0x09;
    const octet TK_FLOAT64 = 0x0A;
    const octet TK_FLOAT128 = 0x0B;
    const octet TK_INT8 = 0x0C;
    const octet TK_UINT8 = 0x0D;
    const octet TK_CHAR8 = 0x10;
    const octet TK_CHAR16 = 0x11;

    // String TKs
    const octet TK_STRING8 = 0x20;
    const octet TK_STRING16 = 0x21;

    // Constructed/Named types
    const octet TK_ALIAS = 0x30;

    // Enumerated TKs
    const octet TK_ENUM = 0x40;
    const octet TK_BITMASK = 0x41;

    // Structured TKs
    const octet TK_ANNOTATION = 0x50;
    const octet TK_STRUCTURE = 0x51;
    const octet TK_UNION = 0x52;
    const octet TK_BITSET = 0x53;

    // Collection TKs
    const octet TK_SEQUENCE = 0x60;
    const octet TK_ARRAY = 0x61;
    const octet TK_MAP = 0x62;

    typedef octet EquivalenceKind;
    const octet EK_MINIMAL = 0xF1;  // 0x1111 0001
    const octet EK_COMPLETE = 0xF2; // 0x1111 0010
    const octet EK_BOTH = 0xF3;     // 0x1111 0011

    typedef octet TypeIdentiferKind;
    const octet TI_STRING8_SMALL = 0x70;
    const octet TI_STRING8_LARGE = 0x71;
    const octet TI_STRING16_SMALL = 0x72;
    const octet TI_STRING16_LARGE = 0x73;
    const octet TI_PLAIN_SEQUENCE_SMALL = 0x80;
    const octet TI_PLAIN_SEQUENCE_LARGE = 0x81;
    const octet TI_PLAIN_ARRAY_SMALL = 0x90;
    const octet TI_PLAIN_ARRAY_LARGE = 0x91;
    const octet TI_PLAIN_MAP_SMALL = 0xA0;
    const octet TI_PLAIN_MAP_LARGE = 0xA1;
    const octet TI_STRONGLY_CONNECTED_COMPONENT = 0xB0;
    const std::string TKNAME_BOOLEAN = "bool";
    const std::string TKNAME_INT16 = "int16_t";
    const std::string TKNAME_UINT16 = "uint16_t";
    const std::string TKNAME_INT32 = "int32_t";
    const std::string TKNAME_UINT32 = "uint32_t";
    const std::string TKNAME_INT64 = "int64_t";
    const std::string TKNAME_UINT64 = "uint64_t";
    const std::string TKNAME_CHAR8 = "char";
    const std::string TKNAME_BYTE = "octet";
    const std::string TKNAME_INT8 = "int8_t";
    const std::string TKNAME_UINT8 = "uint8_t";
    const std::string TKNAME_CHAR16 = "wchar";
    const std::string TKNAME_CHAR16T = "wchar_t";
    const std::string TKNAME_FLOAT32 = "float";
    const std::string TKNAME_FLOAT64 = "double";
    const std::string TKNAME_FLOAT128 = "longdouble";
    const std::string TKNAME_STRING8 = "string";
    const std::string TKNAME_STRING16 = "wstring";

    const std::string TINAME_STRING8_SMALL = "TINAME_STRING8_SMALL";
    const std::string TINAME_STRING8_LARGE = "TINAME_STRING8_LARGE";
    const std::string TINAME_STRING16_SMALL = "TINAME_STRING16_SMALL";
    const std::string TINAME_STRING16_LARGE = "TINAME_STRING16_LARGE";
}
