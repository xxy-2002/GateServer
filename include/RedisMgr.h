#include "singleton.h"
#include "RedisPool.h"
#include "const.h"
#include <memory>

class RedisMgr : public Singleton<RedisMgr> 
{
friend class Singleton<RedisMgr>;

public:
    ~RedisMgr();//析构函数公有的 可以在其他地方释放资源
    bool Get(const std::string &key, std::string& value);
    bool Set(const std::string &key, const std::string &value);
    bool Auth(const std::string &password);
    bool LPush(const std::string &key, const std::string &value);
    bool LPop(const std::string &key, std::string& value);
    bool RPush(const std::string& key, const std::string& value);
    bool RPop(const std::string& key, std::string& value);
    bool HSet(const std::string &key, const std::string  &hkey, const std::string &value);
    bool HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen);
    std::string HGet(const std::string &key, const std::string &hkey);
    bool Del(const std::string &key);
    bool ExistsKey(const std::string &key);
    void Close();
private:
    RedisMgr();
    std::unique_ptr<RedisConPool> _con_pool;
};
