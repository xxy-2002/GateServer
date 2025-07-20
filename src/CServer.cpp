#include "CServer.h"
#include "HttpConnection.h"
#include "AsioIOServicePool.h"
#include <memory>

CServer::CServer(boost::asio::io_context& ioc, unsigned short & port):
    _ioc(ioc), //使用成员初始化列表将传入的 ioc 赋值给类的成员变量 _ioc
    _acceptor(ioc,tcp::endpoint(tcp::v4(),port)),
    _socket(ioc)//初始化 _socket 对象，它是一个 TCP 套接字，用于与客户端进行通信，使用传入的 ioc 进行初始化。
{
    
}
//实现一个Start函数
void CServer::Start()
{   
    auto self = shared_from_this(); //获取当前对象的 shared_ptr 指针，用于在异步操作中传递对象的所有权。 防止析构
    auto& ioc_context = AsioIOServicePool::GetInstance()->GetIOService();
    std::shared_ptr<HttpConnection> new_con = std::make_shared<HttpConnection>(ioc_context);
    _acceptor.async_accept(new_con->GetSocket(),[self,new_con](beast::error_code ec){
        try{
                //出错放弃链接，继续监听其他链接
            if(ec){
                self->Start();//
                std::cout << "accept error:" << ec.message() << std::endl;
                return;
            }
            // //创建新链接并且创建来管理这个来链接
            // std::make_shared<HttpConnection>(std::move(self->_socket))->Start();
            new_con->Start();
            //继续监听
            self->Start();
        }
        catch(std::exception& exp){
            std::cout << "exption is "<<exp.what() << std::endl;
            self->Start();
        }
    });
}
