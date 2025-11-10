
#include "query_redis.hpp"
#include "reg_manager.hpp"
#include <string.h>

int test1()
{
    auto query_redis = std::make_shared<QueryRedis>();
    query_redis->Init();

    uint32_t interval = 0;

    query_redis->AddData("test", 100);
    query_redis->QueryInterval("test", interval);

    query_redis->AddData("test", 100);
    query_redis->QueryInterval("test", interval);

    query_redis->AddData("test", 200);
    query_redis->QueryInterval("test", interval);

    query_redis->AddData("test", 200);
    query_redis->QueryInterval("test", interval);

    std::this_thread::sleep_for(std::chrono::seconds(5));
    query_redis->DeleteData("test", 200);
    query_redis->QueryInterval("test", interval);

    query_redis->DeleteData("test", 100);
    query_redis->QueryInterval("test", interval);

    query_redis->DeleteData("test", 100);
    query_redis->QueryInterval("test", interval);

    std::this_thread::sleep_for(std::chrono::seconds(5));
    query_redis->Stop();

    return 0;
}

int test_reg_manager()
{
    auto reg_manager = std::make_shared<RegManager>();
    return 0;
}

int main()
{
    // test_reg_manager();
    // test1();

    std::vector<TagkeyInfoPtr> tagkey_infos;
    tagkey_infos.reserve(1000 * 1000);
    tagkey_infos[0] = std::make_shared<TagkeyInfo>();
    tagkey_infos[10] = std::make_shared<TagkeyInfo>();
    tagkey_infos[100] = std::make_shared<TagkeyInfo>();
    tagkey_infos[1000] = std::make_shared<TagkeyInfo>();
    tagkey_infos[10000] = std::make_shared<TagkeyInfo>();
    tagkey_infos[100000] = std::make_shared<TagkeyInfo>();
    tagkey_infos[999999] = std::make_shared<TagkeyInfo>();

    std::this_thread::sleep_for(std::chrono::seconds(1000));

    return 0;
}
