#ifndef HTTP_CLIENT_ASYNC(SSL)
#define HTTP_CLIENT_ASYNC(SSL)

#include <iostream>
#include <string>
#include <fstream>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <openssl/ssl.h>
#include <openssl/err.h>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

using readCallback = std::function<void(const std::string&)>;

class session : public std::enable_shared_from_this<session>
{
    private:
        tcp::resolver resolver_;
        beast::ssl_stream<beast::tcp_stream> stream_;
        beast::flat_buffer buffer_;
        http::request<http::empty_body> req_;
        http::response<http::string_body> res_;
        readCallback on_data_received_;

    public:
        session(net::io_context& ioc, ssl::context& ctx, readCallback cb);                                
        void run(char const* host, char const* port, char const* target);
        void on_resolve(beast::error_code ec, tcp::resolver::results_type results);
        void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type);
        void on_handshake(beast::error_code ec);
        void on_write(beast::error_code ec, std::size_t);
        void on_read(beast::error_code ec, std::size_t);
        void on_shutdown(beast::error_code ec);
};

#endif // HTTP_CLIENT_ASYNC(SSL)