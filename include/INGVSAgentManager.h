/***
 * @Author       : yanli yanli563730@baosight.com
 * @Date         : 2024-09-26 17:30:26
 * @FilePath     : /PF/COM/dsf/include/ForDSF/INGVSAgentManager.h
 * @Copyright (c) 2024 by BAOSIGHT, All Rights Reserved.
 * @Description  :
 */
#ifndef H_COM_dsf_include_ForDSF_INGVSAgentManager_H
#define H_COM_dsf_include_ForDSF_INGVSAgentManager_H
#include <string>
#include <map>
#include <vector>
#include <list>

using namespace std;
class RecvDataCallback
{
public:
    virtual void RecvData(string ngvsName, unsigned char *buf, uint32_t length) = 0;
};
class INGVSAgentManager
{
public:
    virtual ~INGVSAgentManager(){};
    virtual bool Initial(const string &filePath, map<string, string> &sendNGVS, map<string, string> &recvNGVS) = 0;
    virtual bool Publish(const string &ngvsIdentifier, unsigned char *buf, uint32_t length) = 0;
    virtual void StartAll() = 0;
    virtual bool StartNGVS(const string &ngvsIdentifier) = 0;
    virtual void StopAll() = 0;
    virtual bool StopNGVS(const string &ngvsIdentifier) = 0;
    virtual bool RegisterCallback(const list<string> &ngvsIdentifiers, RecvDataCallback *callback) = 0;
    virtual void RegisterAll(RecvDataCallback *callback) = 0;
    virtual bool CreateNGVS(const list<string> &filePath, list<string> &ngvsIdentifier) = 0;
    virtual bool DeleteNGVS(const list<string> &ngvsIdentifier) = 0;
    virtual bool ComputeRNHashValue(const std::string &filePath, map<string, string> &RNNGVSHashMap) = 0;
};

class ServiceProvider
{
public:
    static INGVSAgentManager *CreateManager();
    static void DestroyManager(INGVSAgentManager *&manager);
    static string GetSDKProjectVersion();
};
#endif // !H_COM_dsf_include_ForDSF_INGVSAgentManager_H
