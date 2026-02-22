#include "http_client_async_ssl.h"

session_ssl::session_ssl(net::io_context& ioc, std::shared_ptr<ssl::context> ctx, readCallback cb) : ioc_(ioc), ctx_(ctx), resolver_(net::make_strand(ioc)), on_data_received_(std::move(cb))
{
   
}

void session_ssl::run(char const* host, char const* port, char const* target)
{
    last_host_ = host;
    stream_=std::make_unique<beast::ssl_stream<beast::tcp_stream>>(net::make_strand(ioc_),*ctx_);

    if (!SSL_set_tlsext_host_name(stream_->native_handle(), host)) return;

    req_ = {};
    res_ = {};
    buffer_.consume(buffer_.size());

    req_.version(11);
    req_.method(http::verb::get);
    req_.target(target);
    req_.set(http::field::host, host);
    req_.set(http::field::user_agent, "BoostBeastClient2026");

    resolver_.async_resolve(host, port,
        beast::bind_front_handler(&session_ssl::on_resolve, shared_from_this()));
}

void session_ssl::on_resolve(beast::error_code ec, tcp::resolver::results_type results)
{
    if (ec) return; // Обработка ошибки
    beast::get_lowest_layer(*stream_).async_connect(results,beast::bind_front_handler(&session_ssl::on_connect, shared_from_this()));
}

void session_ssl::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type)
{
    if (ec) return; // Обработка ошибки
    stream_->async_handshake(ssl::stream_base::client,beast::bind_front_handler(&session_ssl::on_handshake, shared_from_this()));
}

void session_ssl::on_handshake(beast::error_code ec)
{
    if (ec) return; // Обработка ошибки
    http::async_write(*stream_, req_, beast::bind_front_handler(&session_ssl::on_write, shared_from_this()));
}

void session_ssl::on_write(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);
    if (ec) return; // Обработка ошибки
    res_ = {}; // Очищаем старый ответ
    http::async_read(*stream_, buffer_, res_,beast::bind_front_handler(&session_ssl::on_read, shared_from_this()));
}

void session_ssl::on_read(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);
    if (ec)
    {
        std::cout << "Error!!!!" << std::endl;
        return; 
    }
  
    // Обработка редиректа.Обработка ответов с кодами 3xx
    if (res_.result_int() >= 300 && res_.result_int() < 400 && redirect_limit > 0)
    {
       
        std::string current_url_ = res_[http::field::location];
        if (!current_url_.empty())
        {
            redirect_limit--;
            auto url = boost::urls::parse_uri_reference(current_url_);
            if (url)
            {
                std::string next_target = static_cast<std::string>(url->encoded_resource());
                if (next_target.empty()) next_target = "/";
                std::string next_host = url->has_authority() ? url->host() : last_host_;
                std::string next_port = url->has_port() ? url->port() : "443";

                // Закрываем текущий сокет 
                beast::error_code ec_close;
                beast::get_lowest_layer(*stream_).close();

                // Перезапуск 
                this->run(next_host.c_str(), next_port.c_str(), next_target.c_str());
                return; 
            }
        }
    }

    // Успешная обработка
    if (res_.result_int() >= 200 && res_.result_int() < 300)
    {
        on_data_received_(std::move(res_.body()));
    }
    else
    {
       // std::cout << "Error " << res_.result_int() << " (" << res_.result() << ")" << std::endl;
    }
    //on_data_received_(std::move(res_.body()));
    stream_->async_shutdown(beast::bind_front_handler(&session_ssl::on_shutdown, shared_from_this()));
}

void session_ssl::on_shutdown(beast::error_code ec)
{
    boost::ignore_unused(ec);
}
