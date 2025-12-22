/*
 * @Author      : wengjiqing wengjiqing@baosight.com
 * @Date        : 2024-09-05 14:32:32
 * @FilePath    : /TXDDS/include/RTPS/common/LocatorList_t.h
 * Copyright (c) 2024 by BAOSIGHT, All Rights Reserved.
 * @Description :
 */
#ifndef TXDDS_RTPS_LOCATORLIST_T_H
#define TXDDS_RTPS_LOCATORLIST_T_H

#include "txdds/RTPS/common/Locator.h"

#include <vector>

namespace BaoSky::rtps
{
    using LocatorListConstIterator = std::vector<Locator>::const_iterator;

    class LocatorList_t
    {
    public:
        LocatorList_t();

        virtual ~LocatorList_t();

        inline void Clear()
        {
            mLocators.clear();
        }

        inline uint32_t GetSize()
        {
            return mLocators.size();
        }

        inline LocatorListConstIterator begin() const
        {
            return mLocators.begin();
        }

        inline LocatorListConstIterator end() const
        {
            return mLocators.end();
        }

        bool empty()
        {
            return mLocators.empty();
        }
        
        void push_back(const Locator &loc)
        {
            bool already = false;
            for (auto it = this->begin(); it != this->end(); ++it)
            {
                if (loc == *it)
                {
                    already = true;
                    break;
                }
            }
            if (!already)
            {
                mLocators.push_back(loc);
            }
        }

        void push_back(
            const LocatorList_t &locList)
        {
            for (auto it = locList.mLocators.begin(); it != locList.mLocators.end(); ++it)
            {
                this->push_back(*it);
            }
        }

        std::vector<Locator> mLocators;
    };
}

#endif