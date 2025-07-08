#pragma once
#include "const.h"
#include "logicsystem.h"    
class HttpConnection:public std::enable_shared_from_this<HttpConnection>
{
    friend class LogicSystem; // 声明 LogicSystem 为 HttpConnection 的友元类
    public:
    HttpConnection(tcp::socket socket);
    void Start();
private:
    void CheckDeadline();//检查超时
    void WriteResponse();//写响应
    void HandleReq();//处理请求
    tcp::socket _socket;
    beast::flat_buffer _buffer{8192};//缓冲区
    http::request<http::dynamic_body> _request;//请求
    http::response<http::dynamic_body> _response;//响应 动态
    net::steady_timer deadline_{
        _socket.get_executor(),
        std::chrono::seconds(60)
    };//超时定时器

};
