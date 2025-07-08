#include "HttpConnection.h"

HttpConnection::HttpConnection(tcp::socket socket):_socket(std::move(socket))//socket 移动语义构造的原因
{  
}
void HttpConnection::Start()
{
    auto self = shared_from_this();
    http::async_read(_socket,_buffer,_request,
        [self](beast::error_code ec,std::size_t bytes_transferred){//此处的四个参数可以根据async_read的源码来明白
            try{
                if(ec){
                    std::cout << "read error:" << ec.what() << std::endl;
                    return;
                    //为什么ec可以当成bool来使用
                }

                boost::ignore_unused(bytes_transferred);
                self->HandleReq();
                self->CheckDeadline();
            }
            catch(std::exception& exp){
                std::cout <<"exception is" << exp.what() << std::endl;
            }
        });
}
void HttpConnection::HandleReq()
{
    //设置版本
    _response.version(_request.version());
    //设置短链接
    _response.keep_alive(false);
    if(_request.method() == http::verb::get){
        //设置响应体
       bool success =  LoginSystem::GetInstance()->HandleGet(_request.target(),shared_from_this()); //
        //shared_from_this()：由于 HttpConnection 类继承自 std::enable_shared_from_this<HttpConnection>，shared_from_this() 方法会返回一个指向当前 
        //HttpConnection 对象的 std::shared_ptr，将其作为参数传递给 HandleGet 方法，方便 LoginSystem 与当前连接进行交互。
        if(!success){
            _response.result(http::status::not_found);
            _response.set(http::field::content_type,"text/plain"); //设置回包类型
            beast::ostream(_response.body()) << "url 404 Not Found\r\n"; 
            WriteResponse();//写回包
            return;
        }
        _response.result(http::status::ok);
        _response.set(http::field::server,"xxy_chat"); //xxy_chat需要进行修改，表明处理请求的服务器软件为 xxy_chat。注释提示需要对这个值进行修改，可能是为了使用更准确的服务器软件名称。
        WriteResponse();//写回包
        return;
    }
    //设置状态码
    _response.result(http::status::ok);
    //设置响应体
    _response.set(http::field::content_type,"text/html");
    _response.body() = "hello world";
}

void HttpConnection::WriteResponse(){
    auto self = shared_from_this();
    //http
    _response.content_length(_response.body().size());//设置响应体的长度
    http::async_write(_socket,_response,
        [self](beast::error_code ec,std::size_t bytes_transferred){    //回调函数的作用 1.回调函数是传递给 http::async_write 的 lambda 表达式，其核心作用是在异步操作完成后执行特定逻辑。
            // 此处又没必要try catch 因为async_write 本身就会捕获异常
            self->_socket.shutdown(tcp::socket::shutdown_send,ec); //关闭发送端,只关闭一端
            self->_deadline.cancel();//取消定时器
        });
    
}

void HttpConnection::CheckDeadline()
{
    auto self = shared_from_this();
    _deadline.async_wait(
        [self](beast::error_code ec)
        {
            if(!ec){
                self->_socket.close(ec);
            }
        });
    //定时器的作用防止长时间占用资源
    //不主动关闭客户端：会导致time_wait 状态
}
