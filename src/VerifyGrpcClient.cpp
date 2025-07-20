#include "VerifyGrpcClient.h"
#include "const.h"
#include "ConfigMgr.h"

// 构造函数
// 初始化线程池，设置线程数量为5，连接到指定的主机和端口
VerifyGrpcClient::VerifyGrpcClient() {
    auto& gCfgMgr = ConfigMgr::Inst();
    std::string host = gCfgMgr["VarifyServer"]["Host"];
    std::string port = gCfgMgr["VarifyServer"]["Port"];
    pool_.reset(new RPConPool(5, host, port));
}
//