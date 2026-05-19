#ifndef HTTP_SERVER_ASYNC
#define HTTP_SERVER_ASYNC

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <iostream>
#include "data_base.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

// чтение запроса, его обработка и отправка ответа
// Сессия: логика обработки одного соединения
class session : public std::enable_shared_from_this<session>  // enable_shared_from_this - для обеспечения существования 
{
    beast::tcp_stream stream_;               // TCP-сокет 
    beast::flat_buffer buffer_;              // Буфер для хранения промежуточных данных
    http::request<http::string_body> req_;   // Объект HTTP - запроса, где сообщение - обычная строка
    http::response<http::string_body> res_;  // для строк
    http::response<http::file_body> file_res_;  // для файлов
    int search_results{};
    int min_word_length{};
    int max_word_length{};
    Data_base* db{ nullptr };


public:
    session(tcp::socket&& socket, int _search_results, int _min_word_length, int _max_word_length, Data_base* _db_ptr);
    void run();
private:
    void on_read(beast::error_code ec, std::size_t bytes_transformed);
    //void handle_request(http::request<http::string_body>&& req, http::response<http::string_body>& res);
    void handle_request();
    void on_write(beast::error_code ec, std::size_t bytes_transferred);
    void do_read();
    void send_error(http::status status, std::string text);
};

// Слушатель: принимает новые подключения
class listener : public std::enable_shared_from_this<listener>
{
    net::io_context& ioc_;   // контекст ввода-вывода (управляет очередью событий и взаимодействует с операционной системой)
    tcp::acceptor acceptor_; // объект, который «слушает» входящие TCP-соединения на определенном порту
    int search_results{};
    int min_word_length{};
    int max_word_length{};
    Data_base* db{ nullptr };

public:
    listener(net::io_context& ioc, tcp::endpoint endpoint, int _search_results, int _min_word_length, int _max_word_length, Data_base* _db_ptr);
    void run();
//private:
    void do_accept();
    void on_accept(beast::error_code ec, tcp::socket socket);
};

#endif