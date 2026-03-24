#include "db_loader.h"

#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace data_attr
{
inline bool CheckExtraType(const uint32_t typeId, TypeExtra &typeExtra)
{
    switch (typeId)
    {
    case (17):
        return typeExtra.strLength > 0;
        break;
    case (26):
        return typeExtra.range.size() > 0 && typeExtra.range[0].start < typeExtra.range[0].end;
        break;
    case (27):
        return typeExtra.modelId > 0;
        break;
    default:
        return true;
    }

    return false;
}

// ============================================================
// 构造 / 析构
// ============================================================
DbLoader::DbLoader(const std::string &host, uint16_t port, const std::string &db, const std::string &user,
                   const std::string &password, const SAClient_t clientType)
    : m_host(host), m_port(port), m_db(db), m_user(user), m_password(password), m_clientType(clientType)
{
    m_conn = new SAConnection();
}

DbLoader::DbLoader(const DbConfig &dbConfig)
{
    m_host = dbConfig.host;
    m_port = dbConfig.port;
    m_db = dbConfig.databaseName;
    m_user = dbConfig.user;
    m_password = dbConfig.password;
    m_clientType = dbConfig.clientType;

    m_conn = new SAConnection();
}

DbLoader::~DbLoader()
{
    Disconnect();
    if (nullptr != m_conn)
    {
        delete m_conn;
        m_conn = nullptr;
    }
}

// ============================================================
// Connect / Disconnect
// ============================================================
int32_t DbLoader::Connect()
{
    try
    {
        // "host/unixsock@dbname"
        // "host,port@dbname"  or default port   "host@dbname"
        std::string connStr = m_host + "," + std::to_string(m_port) + "@" + m_db;

        uint32_t timeoutSec = 5;
        m_conn->setOption("MYSQL_OPT_CONNECT_TIMEOUT") = std::to_string(timeoutSec).c_str();
        m_conn->Connect(SAString(connStr.c_str()), SAString(m_user.c_str()), SAString(m_password.c_str()),
                        m_clientType);
        printf("Connect success: connStr = %s\n", connStr.c_str());
    }
    catch (SAException &x)
    {
        fprintf(stderr, "Connect error: %s\n", static_cast<const char *>(x.ErrText()));
        return -1;
    }
    return 0;
}

int32_t DbLoader::Disconnect()
{
    if (m_conn && m_conn->isConnected())
    {
        try
        {
            m_conn->Disconnect();
        }
        catch (SAException &x)
        {
            fprintf(stderr, "Disconnect error: %s\n", static_cast<const char *>(x.ErrText()));
            return -1;
        }
    }
    return 0;
}

// ============================================================
// 辅助函数
// ============================================================
std::string DbLoader::ReadOptStr(SACommand &cmd, int fieldIndex)
{
    SAField &f = cmd.Field(fieldIndex);
    if (f.isNull())
        return {};
    return static_cast<const char *>(f.asString());
}

uint32_t DbLoader::ReadOptUint32(SACommand &cmd, int fieldIndex)
{
    SAField &f = cmd.Field(fieldIndex);
    if (f.isNull())
        return 0u;
    return static_cast<uint32_t>(f.asULong());
}

int16_t DbLoader::ReadOptInt16(SACommand &cmd, int fieldIndex)
{
    SAField &f = cmd.Field(fieldIndex);
    if (f.isNull())
        return 0;
    return static_cast<int16_t>(f.asShort());
}

int32_t DbLoader::ReadOptInt32(SACommand &cmd, int fieldIndex)
{
    SAField &f = cmd.Field(fieldIndex);
    if (f.isNull())
        return 0;
    return static_cast<int32_t>(f.asLong());
}

double DbLoader::ReadOptDouble(SACommand &cmd, int fieldIndex)
{
    SAField &f = cmd.Field(fieldIndex);
    if (f.isNull())
        return 0.0;
    return f.asDouble();
}

std::vector<uint8_t> DbLoader::ReadBlob(SACommand &cmd, int fieldIndex)
{
    SAField &f = cmd.Field(fieldIndex);
    if (f.isNull())
        return {};

    SABytes blob = f.asBytes();
    const uint8_t *data = reinterpret_cast<const uint8_t *>((const void *)blob);
    size_t len = static_cast<size_t>(blob.GetBinaryLength());
    return std::vector<uint8_t>(data, data + len);
}

// ============================================================
// LoadAll
// ============================================================
int32_t DbLoader::LoadAll(DbCache &cache)
{
    int32_t ret = 0;

    // if (LoadDrivers(cache) != 0)
    //     ret = -1;
    // if (LoadChannels(cache) != 0)
    //     ret = -1;
    // if (LoadDevices(cache) != 0)
    //     ret = -1;
    if (LoadTypes(cache) != 0)
        ret = -1;
    // if (LoadModelGroups(cache) != 0)
    //     ret = -1;
    if (LoadModels(cache) != 0)
        ret = -1;
    if (LoadModelProps(cache) != 0)
        ret = -1;
    if (LoadModelInfos(cache) != 0)
        ret = -1;
    // if (LoadVarGroups(cache) != 0)
    //     ret = -1;
    if (LoadVars(cache) != 0)
        ret = -1;
    if (LoadVarProps(cache) != 0)
        ret = -1;

    return ret;
}

// ============================================================
// T_DD_SM_DRIVER
// ============================================================
int32_t DbLoader::LoadDrivers(DbCache &cache)
{
    try
    {
        SACommand cmd(m_conn, "SELECT DRIVER_ID, DRIVER_NAME, DRIVER_TYPE, DESP "
                              "FROM T_DD_SM_DRIVER");
        cmd.Execute();
        while (cmd.FetchNext())
        {
            auto rec = std::make_shared<DriverRecord>();
            rec->driver_id = static_cast<uint32_t>(cmd.Field(1).asULong());
            rec->driver_name = static_cast<const char *>(cmd.Field(2).asString());
            rec->driver_type = static_cast<const char *>(cmd.Field(3).asString());
            rec->desp = ReadOptStr(cmd, 4);
            cache.drivers[rec->driver_id] = rec;
        }
        return 0;
    }
    catch (SAException &x)
    {
        fprintf(stderr, "LoadDrivers error: %s\n", static_cast<const char *>(x.ErrText()));
        return -1;
    }
}

// ============================================================
// T_DD_SM_CHANNEL
// ============================================================
int32_t DbLoader::LoadChannels(DbCache &cache)
{
    try
    {
        SACommand cmd(m_conn, "SELECT CHANNEL_ID, CHANNEL_NAME, CHANNEL_TYPE, "
                              "DRIVER_ID, CONF, DESP, STATUS "
                              "FROM T_DD_SM_CHANNEL");
        cmd.Execute();
        while (cmd.FetchNext())
        {
            auto rec = std::make_shared<ChannelRecord>();
            rec->channel_id = static_cast<uint32_t>(cmd.Field(1).asULong());
            rec->channel_name = static_cast<const char *>(cmd.Field(2).asString());
            rec->channel_type = static_cast<const char *>(cmd.Field(3).asString());
            rec->driver_id = static_cast<uint32_t>(cmd.Field(4).asULong());
            rec->conf = static_cast<const char *>(cmd.Field(5).asString());
            rec->desp = ReadOptStr(cmd, 6);
            rec->status = static_cast<int8_t>(ReadOptInt16(cmd, 7));
            cache.channels[rec->channel_id] = rec;
        }
        return 0;
    }
    catch (SAException &x)
    {
        fprintf(stderr, "LoadChannels error: %s\n", static_cast<const char *>(x.ErrText()));
        return -1;
    }
}

// ============================================================
// T_DD_SM_DEVICE
// ============================================================
int32_t DbLoader::LoadDevices(DbCache &cache)
{
    try
    {
        SACommand cmd(m_conn, "SELECT DEVICE_ID, DRIVER_ID, DEVICE_NAME, DESP, CONF "
                              "FROM T_DD_SM_DEVICE");
        cmd.Execute();
        while (cmd.FetchNext())
        {
            auto rec = std::make_shared<DeviceRecord>();
            rec->device_id = static_cast<uint32_t>(cmd.Field(1).asULong());
            rec->driver_id = static_cast<uint32_t>(cmd.Field(2).asULong());
            rec->device_name = static_cast<const char *>(cmd.Field(3).asString());
            rec->desp = ReadOptStr(cmd, 4);
            rec->conf = static_cast<const char *>(cmd.Field(5).asString());
            cache.devices[rec->device_id] = rec;
        }
        return 0;
    }
    catch (SAException &x)
    {
        fprintf(stderr, "LoadDevices error: %s\n", static_cast<const char *>(x.ErrText()));
        return -1;
    }
}

// ============================================================
// T_DD_SM_TYPE
// ============================================================
int32_t DbLoader::LoadTypes(DbCache &cache)
{
    try
    {
        SACommand cmd(m_conn, "SELECT TYPE_ID, TYPE_NAME, LENGTH FROM T_DD_SM_TYPE");
        cmd.Execute();
        while (cmd.FetchNext())
        {
            auto rec = std::make_shared<TypeRecord>();
            rec->type_id = static_cast<uint32_t>(cmd.Field(1).asULong());
            rec->type_name = static_cast<const char *>(cmd.Field(2).asString());
            rec->length = ReadOptUint32(cmd, 3);
            cache.types[rec->type_id] = rec;
            cache.types_by_name[rec->type_name] = rec;
        }
        return 0;
    }
    catch (SAException &x)
    {
        fprintf(stderr, "LoadTypes error: %s\n", static_cast<const char *>(x.ErrText()));
        return -1;
    }
}

// ============================================================
// T_DD_SM_MDL_GRP
// ============================================================
int32_t DbLoader::LoadModelGroups(DbCache &cache)
{
    try
    {
        SACommand cmd(m_conn, "SELECT NODE_ID, NODE_NAME, PARENT_ID, NODE_TYPE, "
                              "NAMESPACE "
                              "FROM T_DD_SM_MDL_GRP");
        cmd.Execute();
        while (cmd.FetchNext())
        {
            auto rec = std::make_shared<ModelGroupRecord>();
            rec->node_id = static_cast<uint32_t>(cmd.Field(1).asULong());
            rec->node_name = static_cast<const char *>(cmd.Field(2).asString());
            rec->parent_id = ReadOptUint32(cmd, 3);
            rec->node_type = static_cast<int16_t>(cmd.Field(4).asShort());
            rec->name_space = ReadOptStr(cmd, 5);
            cache.model_groups[rec->node_id] = rec;
        }
        return 0;
    }
    catch (SAException &x)
    {
        fprintf(stderr, "LoadModelGroups error: %s\n", static_cast<const char *>(x.ErrText()));
        return -1;
    }
}

// ============================================================
// T_DD_SM_MDL
// ============================================================
int32_t DbLoader::LoadModels(DbCache &cache)
{
    try
    {
        SACommand cmd(m_conn, "SELECT MODEL_ID, NODE_ID, MODEL_NAME, NAMESPACE, DESP, ALIGH_LENGTH "
                              "FROM T_DD_SM_MDL where 1 =1");
        cmd.Execute();
        while (cmd.FetchNext())
        {
            auto rec = std::make_shared<ModelRecord>();
            rec->model_id = static_cast<uint32_t>(cmd.Field(1).asULong());
            rec->node_id = static_cast<uint32_t>(cmd.Field(2).asULong());
            rec->model_name = static_cast<const char *>(cmd.Field(3).asString());
            rec->name_space = static_cast<const char *>(cmd.Field(4).asString());
            rec->desp = static_cast<const char *>(cmd.Field(5).asString());
            rec->aligh_length = static_cast<const char *>(cmd.Field(6).asString());
            cache.models[rec->model_id] = rec;
        }
        return 0;
    }
    catch (SAException &x)
    {
        fprintf(stderr, "LoadModels error: %s\n", static_cast<const char *>(x.ErrText()));
        return -1;
    }
}

// ============================================================
// T_DD_SM_MDL_PROP
// ============================================================
int32_t DbLoader::LoadModelProps(DbCache &cache)
{
    try
    {
        SACommand cmd(m_conn, "SELECT PROP_ID, MODEL_ID, PROP_NAME, TYPE_ID, "
                              "TYPE_EXTRA, INIT_VALUE, DESP, CONTROL_ENABLE "
                              "FROM T_DD_SM_MDL_PROP order by  MODEL_ID, PROP_ID");
        cmd.Execute();
        while (cmd.FetchNext())
        {
            auto rec = std::make_shared<ModelPropRecord>();
            rec->prop_id = static_cast<uint32_t>(cmd.Field(1).asULong());
            rec->model_id = static_cast<uint32_t>(cmd.Field(2).asULong());
            rec->prop_name = static_cast<const char *>(cmd.Field(3).asString());
            rec->type_id = static_cast<uint32_t>(cmd.Field(4).asULong());
            rec->type_extra = ReadOptStr(cmd, 5);
            rec->init_value = ReadBlob(cmd, 6);
            rec->desp = ReadOptStr(cmd, 7);
            rec->control_enable = ReadOptInt16(cmd, 8);
            cache.model_props[rec->model_id].emplace_back(rec);
        }
        return 0;
    }
    catch (SAException &x)
    {
        fprintf(stderr, "LoadModelProps error: %s\n", static_cast<const char *>(x.ErrText()));
        return -1;
    }
}

// ============================================================
// T_DD_SM_MDL_INFO
// ============================================================
int32_t DbLoader::LoadModelInfos(DbCache &cache)
{
    try
    {
        SACommand cmd(m_conn, "SELECT ID, PARENT_MODEL_ID, PROP_NAME, TYPE_ID, "
                              "INIT_VALUE, ALIAS, DESP, CONTROL_ENABLE "
                              "FROM T_DD_SM_MDL_INFO ORDER BY PARENT_MODEL_ID, ID");
        cmd.Execute();
        while (cmd.FetchNext())
        {
            auto rec = std::make_shared<ModelInfoRecord>();
            rec->id = static_cast<uint32_t>(cmd.Field(1).asULong());
            rec->parent_model_id = static_cast<uint32_t>(cmd.Field(2).asULong());
            rec->prop_name = static_cast<const char *>(cmd.Field(3).asString());
            rec->type_id = static_cast<uint32_t>(cmd.Field(4).asULong());
            rec->init_value = ReadBlob(cmd, 5);
            rec->alias = ReadOptStr(cmd, 6);
            rec->desp = ReadOptStr(cmd, 7);
            rec->control_enable = ReadOptInt16(cmd, 8);
            cache.model_infos[rec->parent_model_id].emplace_back(rec);
        }
        return 0;
    }
    catch (SAException &x)
    {
        fprintf(stderr, "LoadModelInfos error: %s\n", static_cast<const char *>(x.ErrText()));
        return -1;
    }
}

// ============================================================
// T_DD_SM_VAR_GRP
// ============================================================
int32_t DbLoader::LoadVarGroups(DbCache &cache)
{
    try
    {
        SACommand cmd(m_conn, "SELECT NODE_ID, NODE_NAME, PARENT_ID, NODE_TYPE, "
                              "NAMESPACE "
                              "FROM T_DD_SM_VAR_GRP");
        cmd.Execute();
        while (cmd.FetchNext())
        {
            auto rec = std::make_shared<VarGroupRecord>();
            rec->node_id = static_cast<uint32_t>(cmd.Field(1).asULong());
            rec->node_name = static_cast<const char *>(cmd.Field(2).asString());
            rec->parent_id = ReadOptUint32(cmd, 3);
            rec->node_type = static_cast<int16_t>(cmd.Field(4).asShort());
            rec->name_space = ReadOptStr(cmd, 5);
            cache.var_groups[rec->node_id] = rec;
        }
        return 0;
    }
    catch (SAException &x)
    {
        fprintf(stderr, "LoadVarGroups error: %s\n", static_cast<const char *>(x.ErrText()));
        return -1;
    }
}

// ============================================================
// T_DD_SM_VAR
// ============================================================
int32_t DbLoader::LoadVars(DbCache &cache)
{
    try
    {
        SACommand cmd(m_conn, "SELECT TAG_ID, TAG_NAME, NODE_ID, NAMESPACE, TYPE_ID, "
                              "TYPE_EXTRA, IO_ADDR, SCAN_INTV, DEVICE_ID "
                              "FROM T_DD_SM_VAR ORDER BY TAG_ID");
        cmd.Execute();
        while (cmd.FetchNext())
        {
            auto rec = std::make_shared<VarRecord>();
            rec->tag_id = static_cast<uint32_t>(cmd.Field(1).asULong());
            rec->tag_name = static_cast<const char *>(cmd.Field(2).asString());
            rec->node_id = static_cast<uint32_t>(cmd.Field(3).asULong());
            rec->name_space = static_cast<const char *>(cmd.Field(4).asString());
            rec->type_id = static_cast<uint32_t>(cmd.Field(5).asULong());
            rec->type_extra = ReadOptStr(cmd, 6);
            rec->io_addr = ReadOptStr(cmd, 7);
            rec->scan_intv = ReadOptInt32(cmd, 8);
            rec->device_id = ReadOptUint32(cmd, 9);
            cache.vars[rec->tag_id] = rec;
        }
        return 0;
    }
    catch (SAException &x)
    {
        fprintf(stderr, "LoadVars error: %s\n", static_cast<const char *>(x.ErrText()));
        return -1;
    }
}

// ============================================================
// T_DD_SM_VAR_PROP
// ============================================================
int32_t DbLoader::LoadVarProps(DbCache &cache)
{
    try
    {
        SACommand cmd(m_conn, "SELECT PROP_ID, TAG_ID, PROP_NAME, TYPE_ID, "
                              "IO_ADDR, INIT_VALUE, DESP, CONTROL_ENABLE, "
                              "SCAN_INTV, DEVICE_ID, BYTE_ORDER, "
                              "EVT_CTRL_ENABLE, EVT_CTRL_PRIOR, OVERRIDE_ENABLE, OVERRIDE_VALUE, "
                              "MODIFY_TIME, EGU_ENABLE, RAW_HIGH, RAW_LOW, EGU_HIGH, EGU_LOW, "
                              "LIMIT_HIGH, LIMIT_LOW, ZP_ENABLE, ZP_CENT, ZP_THRES, "
                              "EGU_LBL, EGU_OPEN, EGU_CLOSE "
                              "FROM T_DD_SM_VAR_PROP ORDER BY TAG_ID, PROP_ID");
        cmd.Execute();
        while (cmd.FetchNext())
        {
            auto rec = std::make_shared<VarPropRecord>();
            rec->prop_id = static_cast<uint32_t>(cmd.Field(1).asULong());
            rec->tag_id = static_cast<uint32_t>(cmd.Field(2).asULong());
            rec->prop_name = static_cast<const char *>(cmd.Field(3).asString());
            rec->type_id = static_cast<uint32_t>(cmd.Field(4).asULong());
            rec->io_addr = ReadOptStr(cmd, 5);
            rec->init_value = ReadBlob(cmd, 6);
            rec->desp = ReadOptStr(cmd, 7);
            rec->control_enable = static_cast<int16_t>(cmd.Field(8).asShort());
            rec->scan_intv = ReadOptInt32(cmd, 9);
            rec->device_id = ReadOptUint32(cmd, 10);
            rec->byte_order = ReadOptStr(cmd, 11);
            rec->evt_ctrl_enable = ReadOptInt16(cmd, 12);
            rec->evt_ctrl_prior = ReadOptInt32(cmd, 13);
            rec->override_enable = ReadOptInt32(cmd, 14);
            rec->override_value = ReadBlob(cmd, 15);
            rec->modify_time = ReadOptStr(cmd, 16);
            rec->egu_enable = ReadOptInt16(cmd, 17);
            rec->raw_high = ReadOptDouble(cmd, 18);
            rec->raw_low = ReadOptDouble(cmd, 19);
            rec->egu_high = ReadOptDouble(cmd, 20);
            rec->egu_low = ReadOptDouble(cmd, 21);
            rec->limit_high = ReadOptDouble(cmd, 22);
            rec->limit_low = ReadOptDouble(cmd, 23);
            rec->zp_enable = ReadOptInt16(cmd, 24);
            rec->zp_cent = ReadOptDouble(cmd, 25);
            rec->zp_thres = ReadOptDouble(cmd, 26);
            rec->egu_lbl = ReadOptStr(cmd, 27);
            rec->egu_open = ReadOptStr(cmd, 28);
            rec->egu_close = ReadOptStr(cmd, 29);
            cache.var_props[rec->tag_id].emplace_back(rec);
        }
        return 0;
    }
    catch (SAException &x)
    {
        fprintf(stderr, "LoadVarProps error: %s\n", static_cast<const char *>(x.ErrText()));
        return -1;
    }
}
} // namespace data_attr