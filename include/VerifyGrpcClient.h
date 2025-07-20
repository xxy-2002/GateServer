#pragma once
#include <grpcpp/grpcpp.h>
#include <memory>
#include <queue>
#include "message.grpc.pb.h"
#include "const.h"
#include "singleton.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetVarifyReq;
using message::GetVarifyRsp;
using message::VarifyService;
class RPConPool {
public:
    RPConPool(size_t poolSize, std::string host, std::string port)
        : poolSize_(poolSize), host_(host), port_(port), b_stop_(false) {
        for (size_t i = 0; i < poolSize_; ++i) {
            std::shared_ptr<Channel> channel = grpc::CreateChannel(host+":"+port,
                grpc::InsecureChannelCredentials());
            connections_.push(VarifyService::NewStub(channel));
        }
    }
    ~RPConPool() {
        std::lock_guard<std::mutex> lock(mutex_);
        Close();
        while (!connections_.empty()) {
            connections_.pop();
        }
    }
    std::unique_ptr<VarifyService::Stub> getConnection() {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] {
            if (b_stop_) {
                return true;
            }
            return !connections_.empty();
            });
        //如果停止则直接返回空指针
        if (b_stop_) {
            return  nullptr;
        }
        auto context = std::move(connections_.front());
        connections_.pop();//出队
        return context;
    }
    void returnConnection(std::unique_ptr<VarifyService::Stub> context) {
        std::lock_guard<std::mutex> lock(mutex_);//加锁，对队列进行操作
        if (b_stop_) {
            return;
        }
        connections_.push(std::move(context));
        cond_.notify_one();
    }
    void Close() {
        b_stop_ = true;
        cond_.notify_all();
    }
private:
    std::atomic<bool> b_stop_;
    size_t poolSize_;
    std::string host_;
    std::string port_;
    std::queue<std::unique_ptr<VarifyService::Stub>> connections_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

class VerifyGrpcClient:public Singleton<VerifyGrpcClient>
{
    friend class Singleton<VerifyGrpcClient>;
public:
    GetVarifyRsp GetVarifyCode(std::string email) {
        ClientContext context;
        GetVarifyRsp reply;
        GetVarifyReq request;
        request.set_email(email);
        auto stub_ = pool_->getConnection();
        Status status = stub_->GetVarifyCode(&context, request, &reply);
        if (status.ok()) {
            return reply;
        }
        else {
            reply.set_error(ErrorCodes::RPCFailed);
            return reply;
        }
    }
private:
    VerifyGrpcClient();
    std::unique_ptr<RPConPool> pool_;
};