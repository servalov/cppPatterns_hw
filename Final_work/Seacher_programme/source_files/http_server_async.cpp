#include "http_server_async.h"

session::session(tcp::socket&& socket) :stream_(std::move(socket))
{

}
void session::run()
{
	do_read();
}

void session::do_read()
{
	req_ = {};
	stream_.expires_after(std::chrono::seconds(30));
	http::async_read(stream_, buffer_, req_, beast::bind_front_handler(&session::on_read, shared_from_this()));
}

void session::on_read(beast::error_code ec, std::size_t bytes_transformed)
{
	boost::ignore_unused(bytes_transformed);
	if (ec) return;
	//auto res = std::make_shared<http::response<http::string_body>>();
	//handle_request(std::move(req_), *res);
	
	handle_request();
	
}

std::string path_cat(beast::string_view base, beast::string_view result) {
	if (base.empty())
		return std::string(result);
	std::string path(base);
#ifdef BOOST_MSVC
	char constexpr path_separator = '\\';
#else
	char constexpr path_separator = '/';
#endif
	if (path.back() == path_separator)
		path.resize(path.size() - 1);
	path.append(result.data(), result.size());
	return path;
}


//void session::handle_request(http::request<http::string_body>&& req, http::response<http::string_body>& res)
void session::handle_request()
{
	
	/*
	res_.version(req_.version());
	res_.result(http::status::ok);
	res_.set(http::field::server, BOOST_BEAST_VERSION_STRING);
	res_.set(http::field::content_type, "text/plain");
	res_.body() = "Hello world";
	res_.prepare_payload();
	
	*/
	
	// 1. Подготовка пути
	std::string path = path_cat(".", req_.target());
	if (req_.target().back() == '/') path.append("index.html");

	std::cout << "Принятый запрос : " << req_.method_string() << " " << req_.target() << std::endl;

	// Обработка GET 
	if (req_.method() == http::verb::get)
	{
		boost::beast::error_code ec;
		http::file_body::value_type body;
		body.open(path.c_str(), boost::beast::file_mode::read, ec);
		
		if (ec) {
			// Если файла нет, используем текстовый res_
			res_ = {};
			res_.result(http::status::not_found);
			res_.set(http::field::content_type, "text/plain");
			res_.body() = "Ошибка. Файл не найден!!!";
			res_.prepare_payload();

			return http::async_write(stream_, res_,
				beast::bind_front_handler(&session::on_write, shared_from_this()));
		}

		auto const size = body.size();
		file_res_ = {
			std::piecewise_construct,
			std::make_tuple(std::move(body)),
			std::make_tuple(http::status::ok, req_.version())
		};
		
		file_res_.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		file_res_.set(http::field::content_type, "text/html");
		file_res_.content_length(file_res_.body().size());
		file_res_.keep_alive(req_.keep_alive());

		return http::async_write(stream_, file_res_,
			beast::bind_front_handler(&session::on_write, shared_from_this()));
	}

	// Для остальных методов(POST) используем res_
	http::async_write(stream_, res_, beast::bind_front_handler(&session::on_write, shared_from_this()));
}

//void session::on_write(bool close, beast::error_code ec, std::size_t bytes_transferred)
void session::on_write(beast::error_code ec, std::size_t bytes_transferred)
{
	if (ec) return;
	//if (close) return stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

	boost::ignore_unused(bytes_transferred);
	if (!res_.keep_alive()) 
	{
		return stream_.socket().shutdown(tcp::socket::shutdown_send);
	}
	//run();
	do_read(); // Продолжаем для keep-alive
}

listener::listener(net::io_context& ioc, tcp::endpoint endpoint) : ioc_(ioc), acceptor_(net::make_strand(ioc)) // strand для многопоточности
{
	beast::error_code ec;
	acceptor_.open(endpoint.protocol());
	acceptor_.set_option(net::socket_base::reuse_address(true));
	acceptor_.bind(endpoint);

	if (ec) {
		std::cerr << "Ошибка привязки: " << ec.message() << std::endl;
		return;
	}

	acceptor_.listen();
}

void listener::run()
{
	do_accept();
}

void listener::do_accept()
{
	
	std::cout << "Ожидание доступа" << std::endl;
	// Принимаем сокет, привязываем метод on_accept
	acceptor_.async_accept(net::make_strand(ioc_),beast::bind_front_handler(&listener::on_accept, shared_from_this()));
}

void listener::on_accept(beast::error_code ec, tcp::socket socket)
{
	if (!ec) 
	{
		std::make_shared<session>(std::move(socket))->run();
	}
	else
	{
		std::cerr << "Ошибка доступа" << ec.message() << std::endl;
	}
	do_accept(); // Снова слушаем порт
}

