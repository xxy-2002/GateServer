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
unsigned char ToHex(unsigned char x)
{
    return  x > 9 ? x + 55 : x + 48;
}
unsigned char FromHex(unsigned char x)
{
    unsigned char y;
    if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
    else if (x >= '0' && x <= '9') y = x - '0';
    else assert(0);
    return y;
}
std::string UrlEncode(const std::string& str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        //判断是否仅有数字和字母构成
        if (isalnum((unsigned char)str[i]) ||
            (str[i] == '-') ||
            (str[i] == '_') ||
            (str[i] == '.') ||
            (str[i] == '~'))
            strTemp += str[i];
        else if (str[i] == ' ') //为空字符
            strTemp += "+";
        else
        {
            //其他字符需要提前加%并且高四位和低四位分别转为16进制
            strTemp += '%';
            strTemp += ToHex((unsigned char)str[i] >> 4);
            strTemp += ToHex((unsigned char)str[i] & 0x0F);
        }
    }
    return strTemp;
}
std::string UrlDecode(const std::string& str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        //还原+为空
        if (str[i] == '+') strTemp += ' ';
        //遇到%将后面的两个字符从16进制转为char再拼接
        else if (str[i] == '%')
        {
            assert(i + 2 < length);
            unsigned char high = FromHex((unsigned char)str[++i]);
            unsigned char low = FromHex((unsigned char)str[++i]);
            strTemp += high * 16 + low;
        }
        else strTemp += str[i];
    }
    return strTemp;
}
void HttpConnection::PreParseGetParam() {
    // 提取 URI  
    auto uri = _request.target();
    // 查找查询字符串的开始位置（即 '?' 的位置）  
    auto query_pos = uri.find('?');
    if (query_pos == std::string::npos) {
        _get_url = uri;
        return;
    }

    _get_url = uri.substr(0, query_pos);
    std::string query_string = uri.substr(query_pos + 1);
    std::string key;
    std::string value;
    size_t pos = 0;
    while ((pos = query_string.find('&')) != std::string::npos) {
        auto pair = query_string.substr(0, pos);
        size_t eq_pos = pair.find('=');
        if (eq_pos != std::string::npos) {
            key = UrlDecode(pair.substr(0, eq_pos)); // 假设有 url_decode 函数来处理URL解码  
            value = UrlDecode(pair.substr(eq_pos + 1));
            _get_params[key] = value;
        }
        query_string.erase(0, pos + 1);
    }
    // 处理最后一个参数对（如果没有 & 分隔符）  
    if (!query_string.empty()) {
        size_t eq_pos = query_string.find('=');
        if (eq_pos != std::string::npos) {
            key = UrlDecode(query_string.substr(0, eq_pos));
            value = UrlDecode(query_string.substr(eq_pos + 1));
            _get_params[key] = value;
        }
    }
}
void HttpConnection::HandleReq()
{
    //设置版本
    _response.version(_request.version());
    //设置短链接
    _response.keep_alive(false);
    if(_request.method() == http::verb::get){
        //设置响应体
        PreParseGetParam();
       bool success =  LogicSystem::GetInstance()->HandleGet(_get_url,shared_from_this()); //
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
        _response.set(http::field::server,"GateServer"); //xxy_chat需要进行修改，表明处理请求的服务器软件为 xxy_chat。注释提示需要对这个值进行修改，可能是为了使用更准确的服务器软件名称。
        WriteResponse();//写回包
        return;
    }

    //处理post请求
    if(_request.method() == http::verb::post){
        //设置响应体
        PreParseGetParam();
       bool success =  LogicSystem::GetInstance()->HandlePost(_request.target(),shared_from_this()); //
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
        _response.set(http::field::server,"GateServer"); //xxy_chat需要进行修改，表明处理请求的服务器软件为 xxy_chat。注释提示需要对这个值进行修改，可能是为了使用更准确的服务器软件名称。
        WriteResponse();//写回包
        return;
    }
    //设置状态码
    _response.result(http::status::ok);
    //设置响应体
    _response.set(http::field::content_type,"text/html");
    WriteResponse();//写回包
    return;
}

void HttpConnection::WriteResponse(){
    auto self = shared_from_this();
    //http
    _response.content_length(_response.body().size());//设置响应体的长度
    http::async_write(_socket,_response,
        [self](beast::error_code ec,std::size_t bytes_transferred){    //回调函数的作用 1.回调函数是传递给 http::async_write 的 lambda 表达式，其核心作用是在异步操作完成后执行特定逻辑。
            // 此处又没必要try catch 因为async_write 本身就会捕获异常
            self->_socket.shutdown(tcp::socket::shutdown_send,ec); //关闭发送端,只关闭一端
            self->deadline_.cancel();//取消定时器
        });
    
}

void HttpConnection::CheckDeadline()
{
    auto self = shared_from_this();
    deadline_.async_wait(
        [self](beast::error_code ec)
        {
            if(!ec){
                self->_socket.close(ec);
            }
        });
    //定时器的作用防止长时间占用资源
    //不主动关闭客户端：会导致time_wait 状态
}