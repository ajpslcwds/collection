#pragma once

#include "db_structs.h"
#include "sqlapi/SQLAPI.h"
#include <string>
#include <unordered_map>
#include <vector>

// Forward declare SQLAPI++ classes to avoid pulling heavy headers into user code
namespace data_attr
{
// ============================================================
// DbCache - 程序全局缓存，保存从数据库读取的所有表数据
// ============================================================
struct DbCache
{
    // key: driver_id
    std::unordered_map<uint32_t, DriverRecordPtr> drivers;
    // key: channel_id
    std::unordered_map<uint32_t, ChannelRecordPtr> channels;
    // key: device_id
    std::unordered_map<uint32_t, DeviceRecordPtr> devices;
    // key: type_id
    std::unordered_map<uint32_t, TypeRecordPtr> types;
    std::unordered_map<std::string, TypeRecordPtr> types_by_name; // key: type_name
    // key: node_id
    std::unordered_map<uint32_t, ModelGroupRecordPtr> model_groups;
    // key: model_id
    std::unordered_map<uint32_t, ModelRecordPtr> models;
    // key: model_id , vector<prop>
    std::unordered_map<uint32_t, std::vector<ModelPropRecordPtr>> model_props; // model_id , vector<prop>
    std::unordered_map<uint32_t, std::vector<ModelInfoRecordPtr>> model_infos; // model_id , vector<info>
    // key: node_id
    std::unordered_map<uint32_t, VarGroupRecordPtr> var_groups;
    // key: tag_id
    std::unordered_map<uint32_t, VarRecordPtr> vars;
    // key: prop_id
    std::unordered_map<uint32_t, std::vector<VarPropRecordPtr>> var_props; // model_id , vector<info>
};

struct DbConfig
{
    SAClient_t clientType = SA_MySQL_Client;
    std::string host = "127.0.0.1";
    int port = 3306;
    std::string databaseName = "";
    std::string user = "";
    std::string password = "";
};

// ============================================================
// DbLoader - 负责从 db 中读取各表数据并填充 DbCache
// ============================================================
class DbLoader
{
  public:
    /**
     * @brief 构造函数
     * @param host     MySQL 服务器地址
     * @param port     端口（默认 3306）
     * @param db       数据库名
     * @param user     用户名
     * @param password 密码
     */
    DbLoader(const std::string &host, uint16_t port, const std::string &db, const std::string &user,
             const std::string &password, const SAClient_t clientType);
    DbLoader(const DbConfig &dbConfig);

    ~DbLoader();

    /** 建立连接（失败时抛出 SAException） */
    int32_t Connect();

    /** 断开连接 */
    int32_t Disconnect();

    /**
     * @brief 读取全部表，填充缓存
     * @param cache 用于存储数据的缓存对象
     * @return 0 成功，-1 失败
     */
    int32_t LoadAll(DbCache &cache);

    // 也可以单独加载某张表，返回 0 成功，-1 失败
    int32_t LoadDrivers(DbCache &cache);
    int32_t LoadChannels(DbCache &cache);
    int32_t LoadDevices(DbCache &cache);
    int32_t LoadTypes(DbCache &cache);
    int32_t LoadModelGroups(DbCache &cache);
    int32_t LoadModels(DbCache &cache);
    int32_t LoadModelProps(DbCache &cache);
    int32_t LoadModelInfos(DbCache &cache);
    int32_t LoadVarGroups(DbCache &cache);
    int32_t LoadVars(DbCache &cache);
    int32_t LoadVarProps(DbCache &cache);

  private:
    std::string m_host;
    uint16_t m_port;
    std::string m_db;
    std::string m_user;
    std::string m_password;
    SAClient_t m_clientType;

    SAConnection *m_conn{nullptr};

    // 辅助：从 SAField 中读取 BLOB 数据
    static std::vector<uint8_t> ReadBlob(SACommand &cmd, int fieldIndex);
    // 辅助：读取可空字符串（IS_NULL 时返回空串）
    static std::string ReadOptStr(SACommand &cmd, int fieldIndex);
    // 辅助：读取可空 uint32（IS_NULL 时返回 0）
    static uint32_t ReadOptUint32(SACommand &cmd, int fieldIndex);
    // 辅助：读取可空 int16（IS_NULL 时返回 0）
    static int16_t ReadOptInt16(SACommand &cmd, int fieldIndex);
    // 辅助：读取可空 int32（IS_NULL 时返回 0）
    static int32_t ReadOptInt32(SACommand &cmd, int fieldIndex);
    // 辅助：读取可空 double（IS_NULL 时返回 0.0）
    static double ReadOptDouble(SACommand &cmd, int fieldIndex);
};
} // namespace data_attr