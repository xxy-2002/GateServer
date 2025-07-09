// GateServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <iostream>

#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "const.h"
#include "CServer.h"
int main()
{   
    std::cout<<"GateServer start"<<std::endl;
    try
    {
        unsigned short port = static_cast<unsigned short>(8080);//端口
        net::io_context ioc{ 1 };//io上下文初始化默认一个线程，底层
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const boost::system::error_code& error, int signal_number) {

            if (error) {
                return;
            }
            ioc.stop();//异步等待信号量，当信号量触发时，停止io上下文
            });
        std::make_shared<CServer>(ioc, port)->Start();
        std::cout<<"GateServer start listen port:"<<port<<std::endl;
        ioc.run();//轮询起来
    }
    catch (std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;//
    }
}