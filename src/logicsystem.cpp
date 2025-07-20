#include "logicsystem.h"
#include "HttpConnection.h"
#include <boost/beast/core/ostream.hpp>
#include <json/value.h>
#include <memory>
#include <ostream>
#include <string>
#include "VerifyGrpcClient.h"
#include "RedisMgr.h"
#include "const.h"
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
        connection->_response.set(http::field::content_type,"application/json"); //设置回包类型
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
            std::cout<<"failed to parse Json data!_src_root"<<std::endl;
            root["error"] = ErrorCodes::Error_Json;
            std::string jsonstr = root.toStyledString();
            beast::ostream(connection->_response.body())<<jsonstr;
            return true;
        }
        auto email = src_root["email"].asString();
        GetVarifyRsp rsp=VerifyGrpcClient::GetInstance()->GetVarifyCode(email);
        std::cout<<"gRPC response error: "<<rsp.error()<<std::endl;
        std::cout<<"email is "<<email<<std::endl;
        root["error"]=rsp.error();
        root["email"]=email;
        std::string jsonstr = root.toStyledString();
        beast::ostream(connection->_response.body()) << jsonstr;
        return true;
    });
	//day11 注册用户逻辑
	RegPost("/user_register", [](std::shared_ptr<HttpConnection> connection) {
		auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
		std::cout << "receive body is " << body_str << std::endl;
		connection->_response.set(http::field::content_type, "text/json");
		Json::Value root;
		Json::Reader reader;
		Json::Value src_root;
		bool parse_success = reader.parse(body_str, src_root);
		if (!parse_success) {
			std::cout << "Failed to parse JSON data!" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		auto email = src_root["email"].asString();
		auto name = src_root["user"].asString();
		auto pwd = src_root["passwd"].asString();
		auto confirm = src_root["confirm"].asString();
		auto icon = src_root["icon"].asString();

		if (pwd != confirm) {
			std::cout << "password err " << std::endl;
			root["error"] = ErrorCodes::PasswdErr;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		//先查找redis中email对应的验证码是否合理
		std::string  varify_code;
		bool b_get_varify = RedisMgr::GetInstance()->Get(CODEPREFIX+src_root["email"].asString(), varify_code);
		if (!b_get_varify) {
			std::cout << " get varify code expired" << std::endl;
			root["error"] = ErrorCodes::VarifyExpired;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		if (varify_code != src_root["varifycode"].asString()) {
			std::cout << " varify code error" << std::endl;
			root["error"] = ErrorCodes::VarifyCodeErr;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		//查找数据库判断用户是否存在
		int uid = MysqlMgr::GetInstance()->RegUser(name, email, pwd, icon);
		if (uid == 0 || uid == -1) {
			std::cout << " user or email exist" << std::endl;
			root["error"] = ErrorCodes::UserExist;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}
		root["error"] = 0;
		root["uid"] = uid;
		root["email"] = email;
		root ["user"]= name;
		root["passwd"] = pwd;
		root["confirm"] = confirm;
		root["icon"] = icon;
		root["varifycode"] = src_root["varifycode"].asString();
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
