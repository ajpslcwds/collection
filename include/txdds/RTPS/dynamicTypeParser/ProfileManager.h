/*
 * @Author       : baihaoran 948942547@qq.com
 * @Date         : 2025-05-16 13:32:50
 * @FilePath     : /TXDDS/include/RTPS/DynamicTypeBuilder/ProfileManager.h
 * @Copyright (c) 2023 by BAOSIGHT, All Rights Reserved.
 * @Description  :
 */
#ifndef _PROFILE_MANAGER_H_
#define _PROFILE_MANAGER_H_
#include <cstdio>
#include <map>
#include <string>
#include <mutex>

#include "txdds/RTPS/dynamicTypeParser/XMLDynamicParser.h"
#include "txdds/RTPS/dynamicTypeParser/SimpleDynamicParser.h"
#include "txdds/DCPS/dynamicType/DynamicTypeBuilder.h"

namespace BaoSky::rtps::DynamicBuilder
{
    class TypeStructureParser;
    class StructMemberInfo;
    class StructInfo;
    class ProfileManager
    {
    public:
        /*
         * @Description:
         * @param {XMLDocument} &doc
         * @return {*}
         */
        static uint64_t loadSimpleTypeStructure(const std::string &filename);
        /*
         * @Description:
         * @param {XMLDocument} &doc
         * @return {*}
         */
        static uint64_t loadSimpleTypeStructure(TypeStructureParser &parser);
        /*
         * @Description:
         * @param {string} &filename
         * @return {*}
         */
        static XMLP_ret loadXMLFile(const std::string &filename);
        /*
         * @Description:
         * @param {XMLDocument} &doc
         * @return {*}
         */
        static XMLP_ret loadXMLNode(tinyxml2::XMLDocument &doc);
        /*
         * @Description:
         * @param {string} &type_name
         * @return {*}
         */
        static BaoSky::dds::DynamicTypeBuilder * getDynamicTypeByName(const std::string &type_name);
        /*
         * @Description:
         * @return {*}
         */
        static bool insertDynamicTypeByName(const std::string &type_name, BaoSky::dds::DynamicTypeBuilder *type);
        /*
         * @Description:
         * @param {string} &typeName
         * @return {*}
         */
        static XMLP_ret DeleteDynamicTypeBuilder(const std::string &typeName);
        /*
         * @Description:
         * @return {*}
         */
        static void DeleteInstance()
        {
            mXMLfiles.clear();
            sDynamicTypes.clear();
        }

    private:
        static std::mutex mMutex;

        static std::map<std::string, BaoSky::dds::DynamicTypeBuilder*> sDynamicTypes;

        static std::map<std::string, XMLP_ret> mXMLfiles;
    };

}

#endif