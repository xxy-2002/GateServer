#include "RedisPool.h"
#include <mutex>

RedisConPool::RedisConPool(size_t poolSize, const char* host, int port, const char* pwd)
: poolSize_(poolSize), host_(host), port_(port), b_stop_(false){
    for (size_t i = 0; i < poolSize_; ++i) {
        auto* context = redisConnect(host, port);
        if (context == nullptr || context->err != 0) {
            if (context != nullptr) {
                redisFree(context);
            }
            continue;
        }
        auto reply = (redisReply*)redisCommand(context, "AUTH %s", pwd);
        if (reply->type == REDIS_REPLY_ERROR) {
            std::cout << "fail to auth" << std::endl;
            //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
            freeReplyObject(reply);
            continue;
        }
        //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
        freeReplyObject(reply);
        std::cout << "sucess to auth" << std::endl;
        connections_.push(context);
    }
} 
RedisConPool::~RedisConPool() {
    std::lock_guard<std::mutex> lock(mutex_);
    while (!connections_.empty()) {
        connections_.pop();
        //释放连接,池里的连接都是由redisConnect创建的,所以需要释放连接,而不是释放redisContext对象
    }
}
redisContext* RedisConPool::getConnection() {
    std::unique_lock<std::mutex> lock(mutex_);//unique_lock是lock_guard的加强版,可以在析构函数中释放锁
    cond_.wait(lock, [this] { 
            if (b_stop_) {
                return true;
            }
            return !connections_.empty(); 
            });//等待条件变量,如果连接池为空,则等待,如果连接池不为空,则返回
    if (b_stop_) {
        return nullptr;
    }
    auto context = connections_.front();//获取连接
    connections_.pop();
    return context;
}
void RedisConPool::returnConnection(redisContext* context) {
    std::lock_guard<std::mutex> lock(mutex_);
    if(b_stop_) {
        redisFree(context);//原因
        return;
    }
    connections_.push(context);
    cond_.notify_one();
}
void RedisConPool::Close() {
    std::lock_guard<std::mutex> lock(mutex_);//加锁,防止多线程同时调用Close函数，若果不加锁，可能会导致多个线程同时调用Close函数，导致连接池被关闭多次
    b_stop_ = true;
    cond_.notify_all();//唤醒所有等待的线程,让它们退出循环
}