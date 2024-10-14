/*  Filename:    fake_ds.h
 *  Copyright:   Shanghai Baosight Software Co., Ltd.
 *
 *  Description: Simulate app to test SDK
 *
 *  @author:     wuzheqiang
 *  @version:    09/10/2024	wuzheqiang	Initial Version
 **************************************************************/

#include "drsdk.h"
#include "drsdk_inner.h"
#include <iostream>

// sync read
void TestReadData()
{
    std::vector<std::string> vecValue;
    std::vector<std::string> vecTagName = {"wzq.read1", "wzq.read2"};
    auto res = drsdk::dr_read_data(vecTagName, &vecValue);
    LOG_INFO << "dr_read_data:res = " << res << std::endl;
    if (0 == res)
    {
        for (int i = 0; i < vecValue.size(); i++)
        {
            LOG_INFO << "dr_read_data: vecTagName:" << vecTagName[i] << ", value:" << vecValue[i] << std::endl;
        }
    }
}

// control cmd
void TestControlData()
{
    std::vector<std::string> vecValue = {"wzq.value1", "wzq.value2", "wzq.value3"};
    std::vector<std::string> vecTagName = {"wzq.control1", "wzq.control2", "wzq.control3"};
    auto res = drsdk::dr_control_data(vecTagName, vecValue);
    LOG_INFO << "dr_control_data:res = " << res << std::endl;
}

// save cmd
void TestSaveData()
{
    std::vector<std::string> vecValue = {"wzq.value1", "wzq.value2", "wzq.value3"};
    std::vector<std::string> vecTagName = {"wzq.control1", "wzq.control2", "wzq.control3"};
    auto res = drsdk::dr_save_data(vecTagName, vecValue);
    LOG_INFO << "dr_save_data:res = " << res << std::endl;
}

// async read
void TestAsyncRegTag()
{
    auto func1 = [](const std::map<std::string, std::string> &mapTagValue) {
        LOG_INFO << "fun_1. mapTagValue.size()=" << mapTagValue.size() << std::endl;
    };
    auto func2 = [](const std::map<std::string, std::string> &mapTagValue) {
        LOG_INFO << "fun_2. mapTagValue.size()=" << mapTagValue.size() << std::endl;
    };
    auto func3 = [](const std::map<std::string, std::string> &mapTagValue) {
        LOG_INFO << "fun_3. mapTagValue.size()=" << mapTagValue.size() << std::endl;
    };

    int32 res = 0;
    res = drsdk::dr_async_reg_tag(std::vector<std::string>{"wzq.reg1", "wzq.reg2"}, 100, func1);
    LOG_INFO << "dr_async_reg_tag1 :res = " << res << std::endl;

    res = drsdk::dr_async_reg_tag(std::vector<std::string>{"wzq.reg2", "wzq.reg3"}, 200, func2);
    LOG_INFO << "dr_async_reg_tag2 :res = " << res << std::endl;

    res = drsdk::dr_async_reg_tag(std::vector<std::string>{"wzq.reg1", "wzq.reg2", "wzq.reg3"}, 50, func3);
    LOG_INFO << "dr_async_reg_tag3 :res = " << res << std::endl;
}

//  read object attr
void TestReadObjectAttr()
{
    std::vector<drsdk::ObjectAttr> vecValue;
    std::string strObjectName = "wzq_object1";
    auto res = drsdk::dr_read_object_attr(strObjectName, &vecValue);
    LOG_INFO << "dr_read_object_attr:res = " << res << std::endl;
    if (0 == res)
    {
        for (int i = 0; i < vecValue.size(); i++)
        {
            LOG_INFO << "dr_read_object_attr:" << vecValue[i].ToString() << std::endl;
        }
    }
}

//  read object  data
void TestReadObjectAttrData()
{

    std::vector<drsdk::ObjectData> vecAttrValues;
    std::string strObjectName = "wzq_object1";
    auto res = drsdk::dr_read_object_data(strObjectName, &vecAttrValues);
    LOG_INFO << "dr_read_object_data:res = " << res << std::endl;
    if (0 == res)
    {
        for (auto &item : vecAttrValues)
        {
            LOG_INFO << "dr_read_object_data: TagName:" << item.sName << ", value:" << item.sValue << std::endl;
        }
    }
}

int main()
{
    // shoud init first
    int32 nRet = drsdk::dr_sdk_init("127.0.0.1", 1234, 1000);
    if (0 != nRet)
    {
        LOG_INFO << "dr_sdk_init failed,nRet=" << nRet << std::endl;
        return -1;
    }

    TestAsyncRegTag();
    while (true)
    { // loop test
        static int cnt1 = 0;
        static int cnt2 = 0;

        if (++cnt1 % 2 == 0)
        {
            cnt1 = 0;
            TestReadData();
            TestControlData();
            TestSaveData();
        }

        if (++cnt2 % 10 == 0)
        {
            cnt2 = 0;
            TestReadObjectAttr();
            TestReadObjectAttrData();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
