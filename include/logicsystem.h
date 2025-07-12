#pragma once
#include "const.h"
#include "singleton.h"
class HttpConnection; // 前置声明
typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler; // 定义HttpHandler类型

class LogicSystem : public Singleton<LogicSystem> {
    friend class Singleton<LogicSystem>; // 友元类，允许Singleton访问私有构造函数
public:
    ~LogicSystem();
    bool HandleGet(std::string,std::shared_ptr<HttpConnection>);//处理get请求
    bool HandlePost(std::string,std::shared_ptr<HttpConnection>);//处理post请求
    void RegGet(std::string,HttpHandler handler);//处理get请求
    void RegPost(std::string url,HttpHandler handler);//处理post请求

private:
    LogicSystem();//构造函数
    // 存储 POST 请求处理函数的映射表
    // 键为字符串类型，通常代表请求的路径
    // 值为 HttpHandler 类型，即一个函数对象，用于处理对应的 POST 请求
    std::map<std::string, HttpHandler> _post_handlers;
    // 存储 GET 请求处理信息的映射表
    // 键为字符串类型，通常代表请求的路径
    std::map<std::string, HttpHandler> _get_handlers;
};

