#include "logicsystem.h"
#include "HttpConnection.h"
//互相包含的解决办法：在其中一个类的头文件中包含另一个类的前置声明，而不是直接包含其头文件。
// 这样可以避免相互依赖的问题，确保两个类都可以被正确编译和使用。   
//在cpp文件中包含.h,解决互相引用
LogicSystem::LogicSystem() {
    ReGet("/get_test",[](std::shared_ptr<HttpConnection> connection){
        beast::ostream(connection->_response.body()) << "receive get_test req";
    });
}
LogicSystem::~LogicSystem() {
}

bool LogicSystem::HandleGet(std::string path, std::shared_ptr<HttpConnection> con) {
    // 查找路径对应的处理函数
    if(_get_handlers.find(path)==_get_handlers.end()){
        return false;
    }
    // 调用处理函数
    _get_handlers[path](con);
    return true;
}
void LogicSystem::ReGet(std::string url, HttpHandler handler) {
    _get_handlers.insert(std::make_pair(url, handler));
}

