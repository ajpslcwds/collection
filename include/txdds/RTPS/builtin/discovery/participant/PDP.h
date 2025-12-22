/*
 * @Author       : songwenguang 563734@baosight.com
 * @Date         : 2024-07-19 17:43:42
 * @FilePath: /TXDDS/include/RTPS/builtin/discovery/participant/PDP.h
 * @Copyright (c) 2023 by BAOSIGHT, All Rights Reserved.
 * @Description  :
 */
#ifndef TXDDS_RTPS_PDP_H
#define TXDDS_RTPS_PDP_H
#include <vector>
#include <atomic>
#include <mutex>
#include <functional>

#include "txdds/RTPS/builtin/discovery/endpoint/EDP.h"
#include "txdds/RTPS/builtin/data/DiscoveredParticipantData.h"
#include "txdds/RTPS/common/Guid.h"
#include "txdds/RTPS/builtin/BuiltinProtocol.h"
#include "txdds/RTPS/timedEvent/ScheduledEvent.h"
#include "txdds/RTPS/builtin/data/DiscoveredWriterData.h"
#include "txdds/RTPS/builtin/data/DiscoveredReaderData.h"
#include "txdds/RTPS/attributes/BuiltinAttributes.h"
#include "txdds/RTPS/participant/IParticipantListener.h"
#include "txdds/DCPS/dynamicType/DynamicTypeParameter.h"

using TypeIdentifier = BaoSky::dds::TypeIdentifier;
using TypeObject = BaoSky::dds::TypeObject;

namespace BaoSky::rtps
{
    class BuiltinProtocol;
    class EDP;
    class IWriter;
    class IWriterHistory;

    class PDP
    {
    public:
        PDP(BuiltinProtocol *builtinProtocol);
        virtual ~PDP();
        /*
         * @description: 根据participant的配置初始化pdp
         * @param {IParticipant} *participant
         * @return {*}
         */
        virtual bool InitPDP(IParticipant *participant);
        /*
         * @description: 根据pdp的builtinAttribute配置DiscoveredParticipantData
         * @param {DiscoveredParticipantData} *discoveredParticipantData
         * @return {*}
         */
        virtual bool InitDiscoveredParticipantData(std::shared_ptr<DiscoveredParticipantData> discoveredParticipantData);
        /*
         * @description: 创建新的DiscoveredParticipantData并添加到vec中
         * @return {*}
         */
        virtual std::shared_ptr<DiscoveredParticipantData> AddDiscoveredParticipantData(const GUID &guid, bool isUseLeaseDuration,
                                                                                        const DiscoveredParticipantData *discoveredParticipantData);
        virtual bool CreatePDPBuiltinEndpoints() = 0;
        virtual void AnnounceParticipantState(bool isNewChange, bool isDispose = false) = 0;
        virtual void AssignRemoteEndpoints(DiscoveredParticipantData *discoveredParticipantData) = 0;
        /*
         * @description: 使能pdp，向外发送公告
         * @return {*}
         */
        virtual bool Enable();
        /*
         * @description: 向外发送公告信息
         * @param {Writer} *writer
         * @param {WriterHistory} *history
         * @param {bool} isDispose
         * @param {bool} isNewChange
         * @return {*}
         */
        virtual void AnnounceParticipantState(IWriter *writer, IWriterHistory *history, bool isDispose, bool isNewChange);
        /*
         * @description: 获取本地participant的DiscoveredParticipantData
         * @return {*}
         */
        std::shared_ptr<DiscoveredParticipantData> GetLocalDiscoveredParticipantData();
        /*
         * @description: 通知discovery新的participant
         * @param {DiscoveredParticipantData} *discoveredParticipantData
         * @param {bool} &isIgnored
         * @return {*}
         */
        virtual void NotifyNewParticipant(DiscoveredParticipantData *discoveredParticipantData, bool &isIgnored);
        /*
         * @description: 创建新的DiscoveredParticipantData
         * @param {DiscoveredParticipantData} *discoveredParticipantData
         * @param {GUID} &writerGuid
         * @return {*}
         */
        virtual std::shared_ptr<DiscoveredParticipantData> CreateParticipantProxyData(DiscoveredParticipantData *discoveredParticipantData, const GUID &writerGuid) = 0;
        virtual bool Init(IParticipant *participant) = 0;
        /*
         * @description: 设置builtin writer的WriterAttributes
         * @param {WriterAttributes} &writerAttributes
         * @return {*}
         */
        virtual bool SetWriterAttributes(WriterAttributes &writerAttributes);
        /*
         * @description: 设置builtin reader的ReaderAttributes
         * @param {ReaderAttributes} &readerAttributes
         * @return {*}
         */
        virtual bool SetReaderAttributes(ReaderAttributes &readerAttributes);
        /*
         * @description: 设置builtin endpoint的locator
         * @param {EndpointAttributes} &endpointAttributes
         * @param {BuiltinProtocol} *builtin
         * @return {*}
         */
        bool SetBuiltinEndpointLocator(EndpointAttributes &endpointAttributes, BuiltinProtocol *builtin);
        /*
         * @description: 获取ParticipantImpl
         * @return {*}
         */
        void ClearPDPResource();
        inline IParticipant *GetParticipant()
        {
            return mParticipant;
        }
        /*
         * @description: 获取锁
         * @return {*}
         */
        inline std::recursive_mutex *GetRemoteMutex()
        {
            return &mRemoteMtx;
        }

        inline std::vector<std::shared_ptr<DiscoveredParticipantData>> &GetDiscoveredParticipantDataVec()
        {
            return mParticipantDiscoveredDataVec;
        }

        inline EDP *GetEDPProtocol()
        {
            return mEDP;
        }

        inline bool GetReadFlag()
        {
            return mReadflag.load();
        }

        inline bool GetAdaptiveAnnouncementFlag()
        {
            return mIsAdaptiveAnnouncement;
        }

        virtual DiscoveredWriterData *AddDiscoveredWriterData(const GUID &writerGuid, GUID &participantGuid,
                                                              std::function<bool(DiscoveredWriterData *, bool, std::shared_ptr<DiscoveredParticipantData>)> iniFunc);
        virtual DiscoveredReaderData *AddDiscoveredReaderData(const GUID &writerGuid, GUID &participantGuid,
                                                              std::function<bool(DiscoveredReaderData *, bool, std::shared_ptr<DiscoveredParticipantData>)> iniFunc);
        virtual void CheckAndNotifyTypeDiscovery(IParticipantListener *listener, const std::string &topicName, const std::string &typeName, const TypeIdentifier &typeIdentifier, const TypeObject &typeObject);
        virtual bool LookupDiscoveredWriterData(const GUID &writerGuid, DiscoveredWriterData &writerData);
        virtual bool LookupDiscoveredReaderData(const GUID &readerGuid, DiscoveredReaderData &readerData);
        virtual bool RemoveDiscoveredWriterData(const GUID &writerGuid);
        virtual bool RemoveDiscoveredReaderData(const GUID &readerGuid);
        virtual void CheckRemoteParticipantLiveliness();
        virtual bool RemoveRemoteParticipant(const GUID &parGuid, ParticipantDiscoveryInfo::DISCOVERY_STATUS reason);
        virtual void UpdateRemoteParticipantLastMsgTime(const GuidPrefix &parGuidPrefix);
        virtual bool RemoveRemoteEndpoint(const DiscoveredParticipantData &remoteParData) = 0;
        virtual bool StopAnnouncement();
        virtual bool ResetAnnouncement();
        virtual void ApplyIniInterval();
        virtual DiscoveredReaderData *CreateDiscoveredReaderData(const GUID &readerGuid, GUID &participantGuid, std::shared_ptr<DiscoveredParticipantData> pardata,
                                                                 std::function<bool(DiscoveredReaderData *, bool, std::shared_ptr<DiscoveredParticipantData>)> iniFunc);
        virtual DiscoveredWriterData *CreateDiscoveredWriterData(const GUID &writerGuid, GUID &participantGuid, std::shared_ptr<DiscoveredParticipantData> pardata,
                                                                 std::function<bool(DiscoveredWriterData *, bool, std::shared_ptr<DiscoveredParticipantData>)> iniFunc);
        virtual DiscoveredReaderData *CreateLocalReaderData(const GUID &readerGuid, GUID &participantGuid,
                                                            std::function<bool(DiscoveredReaderData *, bool, std::shared_ptr<DiscoveredParticipantData>)> iniFunc);
        virtual DiscoveredWriterData *CreateLocalWriterData(const GUID &writerGuid, GUID &participantGuid,
                                                            std::function<bool(DiscoveredWriterData *, bool, std::shared_ptr<DiscoveredParticipantData>)> iniFunc);
        virtual bool DeleteDiscoveredWriterData(const GUID &writerGuid, std::shared_ptr<DiscoveredParticipantData> pData);
        virtual bool DeleteDiscoveredReaderData(const GUID &readerGuid, std::shared_ptr<DiscoveredParticipantData> pData);
        virtual bool DeleteLocalWriterData(const GUID &writerGuid);
        virtual bool DeleteLocalReaderData(const GUID &readerGuid);
        virtual bool RespondAnnouncement(const GuidPrefix &parGuidPrefix, const LocatorList_t &multicastLocatorList, const LocatorList_t &unicastLocatorList);
        virtual bool CheckIsExistSameLocator(const std::vector<Locator> &locatorVec1, const std::vector<Locator> &locatorVec2);

    protected:
        BuiltinProtocol *mBuiltinProtocol;
        IParticipant *mParticipant;
        EDP *mEDP;
        BuiltinAttributes mBuiltinAttributes;
        std::vector<DiscoveredWriterData *> mWriterDiscoveredDataVec;
        std::vector<DiscoveredReaderData *> mReaderDiscoveredDataVec;
        std::vector<std::shared_ptr<DiscoveredParticipantData>> mParticipantDiscoveredDataVec;
        std::shared_ptr<DiscoveredParticipantData> mLocalDiscoveredParticipantData;
        std::atomic_bool mIsEnable;
        ScheduledEvent *mPDPAnnounceEvent;
        ScheduledEvent *mCheckAliveEvent;
        std::recursive_mutex mRemoteMtx;
        std::recursive_mutex mLocalMtx;
        std::mutex mCallbackMtx;
        uint32_t mIniAnnounceNum = 3;
        Duration mIniAnnounceInterval = {0, 50000000u};
        std::atomic<bool> mReadflag{false};
        std::chrono::milliseconds mAdaptiveInterval{150};
        bool mIsAdaptiveAnnouncement = false;
        uint32_t mRespondAnnounceNum = 3;
        std::chrono::milliseconds mRespondAnnounceInterval{50};
        bool mIsStartRespondAnnounce = true;
    };
}

#endif