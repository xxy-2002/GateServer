#include "logicsystem.h"
#include "HttpConnection.h"
#include <boost/beast/core/ostream.hpp>
#include <json/value.h>
#include <memory>
#include <ostream>
#include <string>
//互相包含的解决办法：在其中一个类的头文件中包含另一个类的前置声明，而不是直接包含其头文件。
// 这样可以避免相互依赖的问题，确保两个类都可以被正确编译和使用。   
//在cpp文件中包含.h,解决互相引用
LogicSystem::LogicSystem() {
    RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
        beast::ostream(connection->_response.body()) << "receive get_test req " << std::endl;
        int i = 0;
        for (auto& elem : connection->_get_params) {
            i++;
            beast::ostream(connection->_response.body()) << "param" << i << " key is " << elem.first;
            beast::ostream(connection->_response.body()) << ", " <<  " value is " << elem.second << std::endl;
        }
    });
    RegPost("/get_varifycode",[](std::shared_ptr<HttpConnection> connection){
        auto body_str = beast::buffers_to_string(connection->_request.body().data());//获取请求体
        std::cout<<"receive body is "<<body_str<<std::endl;
        connection->_response.set(http::field::content_type,"text/json"); //设置回包类型
        Json::Value root;
        Json::Reader reader;
        
        Json::Value src_root;
        bool parse_success = reader.parse(body_str,src_root);//解析请求体,解析完放入src_root中
        if(!parse_success){
            std::cout<<"failed to parse Json data!"<<std::endl;
            root["error"] = ErrorCodes::Error_Json;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body())<<jsonstr;
            return true;
        }
        //将json数据加载到root中
        if(!src_root.isMember("email")){
            std::cout<<"failed to parse Json data!"<<std::endl;
            root["error"] = ErrorCodes::Error_Json;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body())<<jsonstr;
            return true;
        }
        auto email = src_root["email"].asString();
        std::cout<<"email is "<<email<<std::endl;
        root["error"]=0;
        root["email"]=src_root["email"];
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->_response.body()) << jsonstr;
        return true;
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
bool LogicSystem::HandlePost(std::string path ,std::shared_ptr<HttpConnection> con){
    if(_post_handlers.find(path)==_post_handlers.end()){
        return false;
    }
    _post_handlers[path](con);
    return true;
}
void LogicSystem::RegGet(std::string url, HttpHandler handler) {
    _get_handlers.insert(std::make_pair(url, handler));
}

void LogicSystem::RegPost(std::string url, HttpHandler handler) {
    _post_handlers.insert(std::make_pair(url, handler));
}
