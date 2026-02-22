#ifndef HTTP_CLIENT_ASYNC_SSL
#define HTTP_CLIENT_ASYNC_SSL

#include <iostream>
#include <string>
#include <fstream>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <boost/url.hpp>
#include <openssl/ssl.h>
#include <openssl/err.h>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

using readCallback = std::function<void(const std::string&)>;

class session_ssl : public std::enable_shared_from_this<session_ssl>
{
    private:
        tcp::resolver resolver_;
        //beast::ssl_stream<beast::tcp_stream> stream_;
        //std::optional<beast::ssl_stream<beast::tcp_stream>> stream_;
        std::unique_ptr<beast::ssl_stream<beast::tcp_stream>> stream_;
        beast::flat_buffer buffer_;
        http::request<http::empty_body> req_;
        http::response<http::string_body> res_;
        readCallback on_data_received_;
        int redirect_limit{ 3 };
        net::io_context& ioc_;        
        std::shared_ptr<ssl::context> ctx_;
        std::string last_host_;

    public:
        session_ssl(net::io_context& ioc, std::shared_ptr<ssl::context> ctx, readCallback cb);
        void run(char const* host, char const* port, char const* target);
        void on_resolve(beast::error_code ec, tcp::resolver::results_type results);
        void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type);
        void on_handshake(beast::error_code ec);
        void on_write(beast::error_code ec, std::size_t bytes_transferred);
        void on_read(beast::error_code ec, std::size_t bytes_transferred);
        void on_shutdown(beast::error_code ec);
};

#endif // HTTP_CLIENT_ASYNC(SSL)