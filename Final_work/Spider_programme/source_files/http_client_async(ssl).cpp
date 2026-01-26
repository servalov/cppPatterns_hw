#include "http_client_async(ssl).h"

session::session(net::io_context& ioc, ssl::context& ctx, readCallback cb) : resolver_(net::make_strand(ioc)), stream_(net::make_strand(ioc), ctx), on_data_received_(std::move(cb))
{

}

void session::run(char const* host, char const* port, char const* target)
{
    // Настройка SNI для корректной работы SSL на practicum.yandex.ru
    if (!SSL_set_tlsext_host_name(stream_.native_handle(), host)) return;

    req_.version(11);
    req_.method(http::verb::get);
    req_.target(target);
    req_.set(http::field::host, host);
    req_.set(http::field::user_agent, "BoostBeastClient2026");

    resolver_.async_resolve(host, port,
        beast::bind_front_handler(&session::on_resolve, shared_from_this()));
}

void session::on_resolve(beast::error_code ec, tcp::resolver::results_type results)
{
    if (ec) return; // Обработка ошибки
    beast::get_lowest_layer(stream_).async_connect(results,beast::bind_front_handler(&session::on_connect, shared_from_this()));
}

void session::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type)
{
    if (ec) return; // Обработка ошибки
    stream_.async_handshake(ssl::stream_base::client,beast::bind_front_handler(&session::on_handshake, shared_from_this()));
}

void session::on_handshake(beast::error_code ec)
{
    if (ec) return; // Обработка ошибки
    http::async_write(stream_, req_, beast::bind_front_handler(&session::on_write, shared_from_this()));
}

void session::on_write(beast::error_code ec, std::size_t) 
{
    if (ec) return; // Обработка ошибки
    http::async_read(stream_, buffer_, res_,beast::bind_front_handler(&session::on_read, shared_from_this()));
}

void session::on_read(beast::error_code ec, std::size_t)
{
    if (ec) return; // Обработка ошибки

    // 1 вариант получения html-кода страницы
    on_data_received_(std::move(res_.body()));
    

    // 2 вариант получения html-кода страницы
    std::vector<std::string> lines_;
    std::stringstream ss(res_.body());
    std::string line;
    while (std::getline(ss, line)) 
    {
        lines_.push_back(std::move(line));
    }

    stream_.async_shutdown(beast::bind_front_handler(&session::on_shutdown, shared_from_this()));
}

void session::on_shutdown(beast::error_code ec)
{

}



