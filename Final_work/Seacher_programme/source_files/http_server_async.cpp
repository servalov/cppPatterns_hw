#include "http_server_async.h"
#include <regex>
#include <string>
#include <boost/locale.hpp>
#include <fstream>
#include <iostream>

// -------------------------------------------------------------------------- 
// объединения двух частей пути
std::string path_cat(beast::string_view base, beast::string_view result)
{
	if (base.empty()) return std::string(result);
	
	std::string path(base);
#ifdef BOOST_MSVC
	char constexpr path_separator = '\\';
#else
	char constexpr path_separator = '/';
#endif
	
	if (path.back() == path_separator)
	{
		path.resize(path.size() - 1);
	}
	
	path.append(result.data(), result.size());
	return path;
}

// метод для определения MIME-типа файла по его расширению
beast::string_view mime_type(beast::string_view path)
{
	using beast::iequals;
	auto const ext = [&path]
	{
		auto const pos = path.rfind(".");
		if (pos == beast::string_view::npos)
			return beast::string_view{};
		return path.substr(pos);
	}();
	if (iequals(ext, ".htm"))  return "text/html";
	if (iequals(ext, ".html")) return "text/html";
	if (iequals(ext, ".php"))  return "text/html";
	if (iequals(ext, ".css"))  return "text/css";
	if (iequals(ext, ".txt"))  return "text/plain";
	if (iequals(ext, ".js"))   return "application/javascript";
	if (iequals(ext, ".json")) return "application/json";
	if (iequals(ext, ".xml"))  return "application/xml";
	if (iequals(ext, ".swf"))  return "application/x-shockwave-flash";
	if (iequals(ext, ".flv"))  return "video/x-flv";
	if (iequals(ext, ".png"))  return "image/png";
	if (iequals(ext, ".jpe"))  return "image/jpeg";
	if (iequals(ext, ".jpeg")) return "image/jpeg";
	if (iequals(ext, ".jpg"))  return "image/jpeg";
	if (iequals(ext, ".gif"))  return "image/gif";
	if (iequals(ext, ".bmp"))  return "image/bmp";
	if (iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
	if (iequals(ext, ".tiff")) return "image/tiff";
	if (iequals(ext, ".tif"))  return "image/tiff";
	if (iequals(ext, ".svg"))  return "image/svg+xml";
	if (iequals(ext, ".svgz")) return "image/svg+xml";
	return "application/text";
}

//Функция очистки строки поиска от служебного содержимого
std::string clear_req_str(const std::string& str)
{
	std::string field_name = "query=";

	if (!str.find(field_name) == 0)
	{
		return "";
	}

	std::string res_str = str;

	res_str.erase(0, field_name.size());

	res_str = std::regex_replace(res_str, std::regex("%09"), " ");  //убрать знаки табуляции
	res_str = std::regex_replace(res_str, std::regex("([\.,:;!?\\\"'*+=_~#$^&])"), " "); //убрать знаки препинания и спец символы
	res_str = std::regex_replace(res_str, std::regex(" {2,}"), " "); //убрать двойные пробелы

	//все строчные
	//std::transform(res_str.begin(), res_str.end(), res_str.begin(),
	//	[](unsigned char c) { return std::tolower(c); });

	// перевод в нижний регистр
	boost::locale::generator gen;                           // Инициализируем генератор локалей
	std::locale loc = gen("ru_RU.UTF-8");
	res_str = boost::locale::to_lower(res_str, loc);   	        // Перевод в нижний регистр с учетом UTF-8

	return res_str;
}

// Функция декодирования
std::string url_decode(std::string_view str)
{
	std::string res;
	res.reserve(str.size());
	for (size_t i = 0; i < str.size(); ++i) {
		if (str[i] == '+') {
			res += ' '; // Заменяем плюсы на пробелы
		}
		else if (str[i] == '%' && i + 2 < str.size()) {
			// Берем две цифры после % и переводим из 16-ричной системы
			int value;
			std::stringstream ss;
			ss << std::hex << str.substr(i + 1, 2);
			ss >> value;
			res += static_cast<char>(value);
			i += 2;
		}
		else {
			res += str[i];
		}
	}
	return res;
}

// Функция кодировки с cp1251_to_utf8
std::string cp1251_to_utf8(const std::string& cp1251)
{
	std::string res;
	int result_u, result_c;
	enum { CP1251 = 1251 };
	result_u = MultiByteToWideChar(CP1251, 0, cp1251.c_str(), -1, 0, 0);
	if (!result_u) {
		throw std::runtime_error("cp1251_to_utf8 cannot convert MultiByteToWideChar!");
	}
	wchar_t* ures = new wchar_t[result_u];
	if (!MultiByteToWideChar(CP1251, 0, cp1251.c_str(), -1, ures, result_u)) {
		delete[] ures;
		throw std::runtime_error("cp1251_to_utf8 cannot convert MultiByteToWideChar 2!");
	}
	result_c = WideCharToMultiByte(CP_UTF8, 0, ures, -1, 0, 0, 0, 0);
	if (!result_c) {
		delete[] ures;
		throw std::runtime_error("cp1251_to_utf8 cannot convert WideCharToMultiByte!");
	}
	char* cres = new char[result_c];
	if (!WideCharToMultiByte(CP_UTF8, 0, ures, -1, cres, result_c, 0, 0)) {
		delete[] cres;
		throw std::runtime_error("cp1251_to_utf8 cannot convert WideCharToMultiByte 2!");
	}
	delete[] ures;
	res.append(cres);
	delete[] cres;
	return res;
}

// Функция кодировки с utf8_to_cp1251
std::string utf8_to_cp1251(std::string const& utf8)
{
	if (!utf8.empty())
	{
		int wchlen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.size(), NULL, 0);
		if (wchlen > 0 && wchlen != 0xFFFD)
		{
			std::vector<wchar_t> wbuf(wchlen);
			int result_u = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.size(), &wbuf[0], wchlen);
			if (!result_u) {
				throw std::runtime_error("utf8_to_cp1251 cannot convert MultiByteToWideChar!");
			}
			std::vector<char> buf(wchlen);
			int result_c = WideCharToMultiByte(1251, 0, &wbuf[0], wchlen, &buf[0], wchlen, 0, 0);
			if (!result_c) {
				throw std::runtime_error("utf8_to_cp1251 cannot convert WideCharToMultiByte!");
			}

			return std::string(&buf[0], wchlen);
		}
	}
	return std::string();
}

// Функция подготовки HTML строки с результатами
std::string prepare_string_to_html_result(const std::string& path, const std::string& query, const std::string& results)
{
	// 1. Читаем исходный файл 
	std::ifstream ifs(path);
	if (!ifs.is_open())
	{
		std::cerr << "Error: Файл " << path << " не найден" << std::endl;
		return cp1251_to_utf8("<html><body><h1> Исходный файл не найден </h1></body></html>");
	}
	// Читаем весь файл в строку content за один проход
	std::string html_start_str((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));

	// 2. Формируем HTML для результатов
	std::string html_results;
	if (results.empty())
	{
		html_results = cp1251_to_utf8("<p> Результатов запроса не найдено.</p>");
	}
	else
	{
		html_results = cp1251_to_utf8("<h3> Результаты запроса (") + query + "):</h3>" + cp1251_to_utf8(results);
	}

	// 3. Заменяем метку в файле на результаты поиска в БД
	std::string marker = "<!--search result below-->";
	size_t pos = html_start_str.find(marker);
	if (pos != std::string::npos)
	{
		html_start_str.replace(pos, marker.length(), html_results);
	}
	else
	{
		// Если метка не найдена, добавляем в конец
		html_start_str += html_results;
	}

	return html_start_str;
}
//--------------------------------------------------------------
// Реализация класса session
session::session(tcp::socket&& socket, int _search_results, int _min_word_length, int _max_word_length, Data_base* _db_ptr) :stream_(std::move(socket)),db(_db_ptr)
{
	search_results = _search_results;
	min_word_length = _min_word_length;
	max_word_length = _max_word_length;
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

	handle_request();
}

// Вспомогательный метод
void session::send_error(http::status status, std::string text)
{
	res_ = { status, req_.version() };
	res_.set(http::field::content_type, "text/plain");
	res_.body() = text;
	res_.prepare_payload();
	http::async_write(stream_, res_, beast::bind_front_handler(&session::on_write, shared_from_this()));
}

// Обработчие GET и POST в handle_request
void session::handle_request()
{
	// 1. Подготовка пути
	std::string path = path_cat(".", req_.target());
	if (req_.target().back() == '/') path.append("index.html");

	std::cout << " Принятый запрос : " << req_.method_string() << " " << req_.target() << std::endl;

	// --- Обработка GET ---- 
	if (req_.method() == http::verb::get)
	{
		boost::beast::error_code ec;
		http::file_body::value_type body;
		body.open(path.c_str(), boost::beast::file_mode::read, ec);

		if (ec) 
		{
			// Если файла нет, используем текстовый res_
			res_ = {};
			res_.result(http::status::not_found);
			res_.set(http::field::content_type, "text/plain");
			res_.body() = " Ошибка. Файл не найден!!!";
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
		// Используем mime_type для корректного отображения стилей и скриптов
		//file_res_.set(http::field::content_type, "text/html");
		file_res_.set(http::field::content_type, mime_type(path));
		file_res_.content_length(file_res_.body().size());
		file_res_.keep_alive(req_.keep_alive());

		return http::async_write(stream_, file_res_,
			beast::bind_front_handler(&session::on_write, shared_from_this()));
	}
	// --- Обработка POST ---
	else if (req_.method() == http::verb::post)
	{
		std::string req_str = clear_req_str(url_decode(req_.body()));
		std::cout << "\n Запрос пользователя: " << utf8_to_cp1251(req_str) << std::endl;

		std::set<std::string> words_str;
		std::string word;
		std::stringstream ss(req_str);
		std::cout << " Отправка запроса на сервер по словам: ";
		while (ss >> word)
		{
			if (word.size() > min_word_length && word.size() < max_word_length)
			{
				std::cout << utf8_to_cp1251(word) << " ; ";
				words_str.insert(word);
			}
		}
		std::cout << std::endl;

		//список адресов, в которых встречаются слова
		std::string urls_list = db->poisk_url_by_words(words_str, search_results);
		
		std::cout << "\n HTML-код страницы следующий:" << std::endl;
		std::cout << urls_list << std::endl;

		http::response<http::string_body> res{ http::status::ok, req_.version() };
		res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(http::field::content_type, "text/html");
		res.keep_alive(req_.keep_alive());
		//Читаем файл, находим метку и заменяем её на сформированный список 
		res.body() = prepare_string_to_html_result(path, req_str, urls_list);
		res.content_length(res.body().size());
		res.prepare_payload();

		res_ = std::move(res);
		return http::async_write(stream_, res_,
			beast::bind_front_handler(&session::on_write, shared_from_this()));
	}

	// Остальные методы
	//http::async_write(stream_, res_, beast::bind_front_handler(&session::on_write, shared_from_this()));
	return send_error(http::status::method_not_allowed, " Неизвестный метод !!!");
}

void session::on_write(beast::error_code ec, std::size_t bytes_transferred)
{
	boost::ignore_unused(bytes_transferred);
	if (ec) return;
	
	if (!res_.keep_alive()) 
	{
		return stream_.socket().shutdown(tcp::socket::shutdown_send);
	}

	do_read(); // Продолжаем для keep-alive
}

// Реализация класса listener
listener::listener(net::io_context& ioc, tcp::endpoint endpoint, int _search_results, int _min_word_length, int _max_word_length, Data_base* _db_ptr) : ioc_(ioc), acceptor_(net::make_strand(ioc)),db(_db_ptr) // strand для многопоточности
{
	
	search_results = _search_results;
	min_word_length = _min_word_length;
	max_word_length = _max_word_length;
	
	beast::error_code ec;
	acceptor_.open(endpoint.protocol(),ec);
	if (ec) { std::cerr << " Ошибка открытия: " << ec.message() << std::endl; return; }

	acceptor_.set_option(net::socket_base::reuse_address(true),ec);
	if (ec) { std::cerr << " Ошибка опции: " << ec.message() << std::endl; return; }

	acceptor_.bind(endpoint,ec);
	if (ec) { std::cerr << " Ошибка привязки: " << ec.message() << std::endl; return; }


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
	
	std::cout << " Ожидание приема входящих запросов ... " << std::endl;
	// Принимаем сокет, привязываем метод on_accept
	acceptor_.async_accept(net::make_strand(ioc_),beast::bind_front_handler(&listener::on_accept, shared_from_this()));
}

void listener::on_accept(beast::error_code ec, tcp::socket socket)
{
	if (!ec) 
	{
		std::make_shared<session>(std::move(socket),search_results, min_word_length, max_word_length,db)->run();
	}
	else
	{
		std::cerr << " Ошибка доступа" << ec.message() << std::endl;
	}
	do_accept(); // Снова слушаем порт
}

