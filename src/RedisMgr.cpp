#include "RedisMgr.h"
#include "ConfigMgr.h"
#include <iostream>
RedisMgr::RedisMgr() {
    auto& gCfgMgr = ConfigMgr::Inst();
    auto host = gCfgMgr["Redis"]["Host"];
    auto port = gCfgMgr["Redis"]["Port"];
    auto pwd = gCfgMgr["Redis"]["Password"];
    _con_pool.reset(new RedisConPool(5, host.c_str(), atoi(port.c_str()), pwd.c_str()));
}
RedisMgr::~RedisMgr() {
    Close();
}

bool RedisMgr::Get(const std::string &key, std::string& value)
{   
    auto connect = _con_pool->getConnection();//获取连接
    if(connect == nullptr) {
        std::cout << "failed" << std::endl;
        return false;
    }
    auto reply = (redisReply*)redisCommand(connect, "GET %s", key.c_str());//执行redis命令行
     reply = (redisReply*)redisCommand(connect, "GET %s", key.c_str());
     if (reply == NULL) {
         std::cout << "[ GET  " << key << " ] failed" << std::endl;
         freeReplyObject(reply);//释放redisReply对象,原因是redisCommand执行后返回的redisReply所占用的内存防止内存泄漏
          return false;
    }
     if (reply->type != REDIS_REPLY_STRING) {//如果返回类型不是字符串 则释放连接
         std::cout << "[ GET  " << key << " ] failed" << std::endl;
         freeReplyObject(reply);
         return false;
    }
     value = reply->str;
     freeReplyObject(reply);//释放redisReply对象,原因是redisCommand执行后返回的redisReply所占用的内存防止内存泄漏
     std::cout << "Succeed to execute command [ GET " << key << "  ]" << std::endl;
    _con_pool->returnConnection(connect);//归还连接
     return true;
}

bool RedisMgr::Set(const std::string &key, const std::string &value){
    //执行redis命令行
    auto connect = _con_pool->getConnection();//获取连接
    if(connect == nullptr) {
        std::cout << "failed" << std::endl;
        return false;
    }
    auto reply = (redisReply*)redisCommand(connect, "SET %s %s", key.c_str(), value.c_str());
    //如果返回NULL则说明执行失败
    if (NULL == reply)
    {
        std::cout << "Execut command [ SET " << key << "  "<< value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    //如果执行失败则释放连接
    if (!(reply->type == REDIS_REPLY_STATUS && (strcmp(reply->str, "OK") == 0 || strcmp(reply->str, "ok") == 0)))//
    {
        std::cout << "Execut command [ SET " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);     
        return false;
    }
    //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
    freeReplyObject(reply);
    std::cout << "Execut command [ SET " << key << "  " << value << " ] success ! " << std::endl;
    _con_pool->returnConnection(connect);//归还连接
    return true;
}

// bool RedisMgr::Auth(const std::string &password)
// {
//     auto connect = _con_pool->getConnection();//获取连接
//     if(connect == nullptr) {
//         std::cout << " failed" << std::endl;
//         return false;
//     }
//     auto reply = (redisReply*)redisCommand(connect, "AUTH %s", password.c_str());
//     if (reply->type == REDIS_REPLY_ERROR) {
//         std::cout << "认证失败" << std::endl;
//         //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
//         freeReplyObject(reply);
//         return false;
//     }
//     else {
//         //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
//         freeReplyObject(reply);
//         std::cout << "认证成功" << std::endl;

//         return true;
//     }
// }
bool RedisMgr::LPush(const std::string &key, const std::string &value) {
    auto connect = _con_pool->getConnection();//获取连接
    if(connect == nullptr) {
        std::cout << "failed" << std::endl;
        return false;
    }
    auto reply = (redisReply*)redisCommand(connect, "LPUSH %s %s", key.c_str(), value.c_str());
    if (reply == NULL) {
        std::cout << "[ LPUSH  " << key << "  " << value << " ] failed" << std::endl;
        freeReplyObject(reply);
        return false;
    }
    if (reply->type != REDIS_REPLY_INTEGER||reply->integer<=0) {//如果返回类型不是整形 则释放连接
        std::cout << "[ LPUSH  " << key << "  " << value << " ] failed" << std::endl;
        freeReplyObject(reply);
        return false;
    }

    freeReplyObject(reply);
    std::cout << "Succeed to execute command [ LPUSH " << key << "  " << value << " ]" << std::endl;
    _con_pool->returnConnection(connect);//归还连接
    return true;
}

bool RedisMgr::LPop(const std::string &key, std::string& value){
    auto connect = _con_pool->getConnection();//获取连接
    if(connect == nullptr) {
        std::cout << "failed" << std::endl;
        return false;
    }
    auto reply = (redisReply*)redisCommand(connect, "LPOP %s ", key.c_str());
    if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
        std::cout << "Execut command [ LPOP " << key<<  " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    value = reply->str;
    std::cout << "Execut command [ LPOP " << key <<  " ] success ! " << std::endl;
    freeReplyObject(reply);
    _con_pool->returnConnection(connect);//归还连接
    return true;
}

bool RedisMgr::RPush(const std::string& key, const std::string& value) {
    auto connect = _con_pool->getConnection();//获取连接
    if(connect == nullptr) {
        std::cout << "failed" << std::endl;
        return false;
    }
    auto reply = (redisReply*)redisCommand(connect, "RPUSH %s %s", key.c_str(), value.c_str());
    if (NULL == reply)
    {
        std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
        std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] success ! " << std::endl;
    freeReplyObject(reply);
    _con_pool->returnConnection(connect);//归还连接
    return true;
}

bool RedisMgr::RPop(const std::string& key, std::string& value) {
    auto connect = _con_pool->getConnection();//获取连接
    if(connect == nullptr) {
        std::cout << "failed" << std::endl;
        return false;
    }
    auto reply = (redisReply*)redisCommand(connect, "RPOP %s ", key.c_str());
    if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
        std::cout << "Execut command [ RPOP " << key << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    value = reply->str;
    std::cout << "Execut command [ RPOP " << key << " ] success ! " << std::endl;
    freeReplyObject(reply);
    _con_pool->returnConnection(connect);//归还连接
    return true;
}
// 向哈希表中添加一个字段和值，如果字段已经存在，则更新其值。
bool RedisMgr::HSet(const std::string &key, const std::string &hkey, const std::string &value) {
    auto connect = _con_pool->getConnection();//获取连接
    if(connect == nullptr) {
        std::cout << "failed" << std::endl;
        return false;
    }
    auto reply = (redisReply*)redisCommand(connect, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER ) {
        std::cout << "Execut command [ HSet " << key << "  " << hkey <<"  " << value << " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << value << " ] success ! " << std::endl;
    freeReplyObject(reply);//为空指针时不需要释放内存，操作完成后会自动释放内存
    _con_pool->returnConnection(connect);//归还连接
    return true;
}

std::string RedisMgr::HGet(const std::string &key, const std::string &hkey)
{
    auto connect = _con_pool->getConnection();//获取连接
    if(connect == nullptr) {
        std::cout << "failed" << std::endl;
        return "";
    }
    const char* argv[3];
    size_t argvlen[3];
    argv[0] = "HGET";
    argvlen[0] = 4;
    argv[1] = key.c_str();
    argvlen[1] = key.length();
    argv[2] = hkey.c_str();
    argvlen[2] = hkey.length();
    auto reply = (redisReply*)redisCommandArgv(connect, 3, argv, argvlen);//执行redis命令行的另一种方式
    if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
        freeReplyObject(reply);
        std::cout << "Execut command [ HGet " << key << " "<< hkey <<"  ] failure ! " << std::endl;
        return "";
    }
    std::string value = reply->str;
    freeReplyObject(reply);
    std::cout << "Execut command [ HGet " << key << " " << hkey << " ] success ! " << std::endl;
    _con_pool->returnConnection(connect);//归还连接
    return value;
}

bool RedisMgr::Del(const std::string &key)
{
    auto connect = _con_pool->getConnection();//获取连接
    if(connect == nullptr) {
        std::cout << "failed" << std::endl;
        return false;
    }
    auto reply = (redisReply*)redisCommand(connect, "DEL %s", key.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
        std::cout << "Execut command [ Del " << key <<  " ] failure ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    std::cout << "Execut command [ Del " << key << " ] success ! " << std::endl;
     freeReplyObject(reply);
     _con_pool->returnConnection(connect);//归还连接
     return true;
}

bool RedisMgr::ExistsKey(const std::string &key)
{
    auto connect = _con_pool->getConnection();//获取连接
    if(connect == nullptr) {
        std::cout << "failed" << std::endl;
        return false;
    }
    auto reply = (redisReply*)redisCommand(connect, "exists %s", key.c_str());
    if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER || reply->integer == 0) {
        std::cout << "Not Found [ Key " << key << " ]  ! " << std::endl;
        freeReplyObject(reply);
        return false;
    }
    std::cout << " Found [ Key " << key << " ] exists ! " << std::endl;
    freeReplyObject(reply);
    _con_pool->returnConnection(connect);//归还连接
    return true;
}

void RedisMgr::Close()
{
    _con_pool->Close();
}