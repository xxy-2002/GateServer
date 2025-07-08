#pragma once //确保该头文件在编译过程中只被包含一次，防止头文件重复包含导致的重复定义错误。
#include "const.h"

class CServer:public std::enable_shared_from_this<CServer> //定义一个CServer类，继承自std::enable_shared_from_this<CServer>，用于在类的成员函数中获取类的shared_ptr指针。   奇异递归模板
{
public:
    CServer(boost::asio::io_context& ioc, unsigned short& port);//构造函数 参数是io上下文和端口
    void Start();//启动函数
private:
    tcp::acceptor  _acceptor;//acceptor 是一个类，用于接受连接
    net::io_context& _ioc;//io上下文
    tcp::socket   _socket;//socket 是一个类，用于表示套接字
};
