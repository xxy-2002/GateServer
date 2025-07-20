#include "const.h"
class RedisConPool{
public:
    RedisConPool(size_t poolSize, const char* host, int port, const char* pwd);
    ~RedisConPool();
    redisContext* getConnection();//获取连接
    void returnConnection(redisContext* context);//归还连接
    void Close();
private:
    std::atomic<bool> b_stop_;
    size_t poolSize_;
    const char* host_;
    const char* pwd_;
    int port_;
    std::queue<redisContext*> connections_;
    std::mutex mutex_;
    std::condition_variable cond_;
};
