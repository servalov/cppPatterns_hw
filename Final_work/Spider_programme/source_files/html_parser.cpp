#include "html_parser.h"

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

// Функция получения path, где лежит страница
std::string Html_parser::get_base_path(const std::string& in_str)
{
	std::string res_str = in_str;
	// 1. Убираем слеш в конце
	res_str = std::regex_replace(res_str, std::regex("/$"), "");

	// 2. Выделяем протокол
	std::string http_prefix{};    // пустой префикс
	if (res_str.find("https://") == 0) http_prefix = "https://";
	else if (res_str.find("http://") == 0) http_prefix = "http://";

	res_str.erase(0, http_prefix.size());      // удаляем префикс

	// 3. Ищем последний сегмент. Если в нем есть точка — удаляем (т.к. это файл)
	std::smatch res;
	std::regex r("/(.[^/]*)$");
	if (std::regex_search(res_str, res, r))
	{
		if (res.str(1).find(".") != std::string::npos)
		{
			res_str.erase(res.position(0), res.length(0));
		}
	}
	// 4. Убираем лишние слеши
	res_str = std::regex_replace(res_str, std::regex("/{2,}"), "/");
	return http_prefix + res_str;
}

// Функция получения полного url
std::string Html_parser::complete_url(const std::string& in_url, const std::string& url_base)
{
	// Если это пустой URL
	if (in_url.empty()) return url_base;

	// Если это уже полный URL (Если in_url уже начинается с http://, https:// или //, функция возвращает его)
	if (in_url.find("://") != std::string::npos || in_url.find("//") == 0)
	{
		return in_url;
	}

	std::string res_url = in_url;
	// Если путь от корня — убираем ведущий слеш для склейки
	if (res_url[0] == '/')
	{
		res_url.erase(0, 1);
	}

	// Склеиваем результат
	std::string full = url_base + "/" + res_url;

	// Убираем двойные слеши
	size_t p_pos = full.find("://");
	std::string p = (p_pos != std::string::npos) ? full.substr(0, p_pos + 3) : "";
	std::string path = (p_pos != std::string::npos) ? full.substr(p_pos + 3) : full;
	path = std::regex_replace(path, std::regex("/{2,}"), "/");

	return p + path;
}

// Получение ссылок текущей страницы
std::vector<std::string> Html_parser::get_all_links(const std::string& html_code, const std::string& current_url)
{
	std::vector<std::string> result;
	std::string base = get_base_path(current_url);      // путь, где лежит страница

	// Поиск тегов <a href="...">. Игнорируем регистр (icase).
	std::regex link_re(R"(<a[^>]*?\s+href\s*=\s*["']([^"']+)["'])", std::regex_constants::icase);

	//Используем итераторы для поиска всех вхождений
	auto it = std::sregex_iterator(html_code.begin(), html_code.end(), link_re);
	auto end = std::sregex_iterator();

	std::set<std::string> unique_links;
	for (; it != end; ++it)
	{
		std::string raw_url = (*it).str(1);
		if (raw_url.empty() || raw_url[0] == '#')
		{
			continue;
		}
		std::string full_url = complete_url(raw_url, base);
		//std::cout << (*it).str(0)<<"   "<< full_url << std::endl;
		
		if (full_url != current_url)
		{
			unique_links.insert(full_url);
		}
	}

	return std::vector<std::string>(unique_links.begin(), unique_links.end());
	//result.push_back(full_url);

	return result;
}

// Получение host и target
void Html_parser::get_host_target(std::string& new_url, std::string& host, std::string& target)
{
	host = new_url;
	target = "/"; // по умолчанию 

	// 1. Извлекаем хост 
	size_t http_prefix_pos = host.find("://");
	if (http_prefix_pos != std::string::npos)
	{
		host.erase(0, http_prefix_pos + 3);
	}

	// 2. Ищем начало пути
	size_t path_pos = host.find('/');
	if (path_pos != std::string::npos) {

		target = host.substr(path_pos);
		host = host.substr(0, path_pos);
	}

	// 3. Удаляем порт из хоста, если он мешает соединению
	size_t port_pos = host.find(':');
	if (port_pos != std::string::npos)
	{
		host = host.substr(0, port_pos);
	}
}

// Индексирование страницы
std::string Html_parser::html_index(const std::string& html_str)
{
	std::string html = html_str;

	html = std::regex_replace(html, std::regex(R"(<\s*/\s*)"), "</");  // всевозможные варианты пробелов в закрывающем теге
	html = std::regex_replace(html, std::regex(R"(<\s+)"), "</");      // убираем пробелы после закрывающей скобки 
	html = std::regex_replace(html, std::regex(">"), "> ");            // наличие пробела между тегами
	html = std::regex_replace(html, std::regex(R"(\s+)"), " ");        // убираем любые последовательности пробелов, табуляции и переноса строк

	// получение текста из заголовка
	std::regex title_regex("<title>(.*?)</title>", std::regex::icase);
	std::smatch match;
	std::string title_str{};

	if (std::regex_search(html, match, title_regex))
	{
		title_str = match[1].str();
	}

	// удаляем все, что выше <body> 
	html = std::regex_replace(html, std::regex(R"(^.*?(?=<body))", std::regex::icase), "");

	// удаляем все, что после <body> 
	html = std::regex_replace(html, std::regex(R"(<\/body>.*$)", std::regex::icase), "");

	html = title_str + " " + html;
	html = std::regex_replace(html, std::regex("<[^>]*>"), "");
	html = std::regex_replace(html, std::regex(R"([.,:;@!~=%&#^|$?/<>(){}|"'*+_\-])"), " ");   // удаление спецсимволов
	html = std::regex_replace(html, std::regex(R"(\[[^\]]*\])"), "");                          // удаление скобок
	//html = std::regex_replace(html, std::regex(R"(\\u\S*)"), "");                            // удаление спецсимволов

	html = std::regex_replace(html, std::regex(R"([a-z]+)", std::regex::icase), " ");          // Удаляем все слова, состоящие из английских букв
	//html = std::regex_replace(html, std::regex(R"(["'«»„“])"), " ");

	html = std::regex_replace(html, std::regex(R"(\d+)"), "");                                 // удаление цифр
	html = std::regex_replace(html, std::regex(R"(\s+)"), " ");                                // убираем любые последовательности пробелов, табуляции и переноса строк

	// перевод в нижний регистр
	boost::locale::generator gen;                           // Инициализируем генератор локалей
	std::locale loc = gen("ru_RU.UTF-8");
	html = boost::locale::to_lower(html, loc);   	        // Перевод в нижний регистр с учетом UTF-8

	return html;
}

// Получение новых слов для добавления их в БД
std::map<std::string, unsigned int> Html_parser::new_words(const std::string& html_str, const std::string& current_url)
{
	std::map<std::string, unsigned int> words_html;
    std::string word;
    std::stringstream ss(html_str);
	
	std::cout << " Слова, полученные c страницы " << current_url << std::endl;
	std::cout << " ---------------------------------------------------- " << std::endl;
    while (ss >> word)
    {
    	int str_len = word.size()/2;


		// для кириллицы
		if (std::regex_match(utf8_to_cp1251(word), std::regex(R"([а-яА-ЯёЁ]+)")))
		{
			if (str_len > min_word_length && str_len < max_word_length)
			{
				words_html[word]++;
				std::cout <<"   "<< utf8_to_cp1251(word) << " (длина: " << str_len << ")" << std::endl;
			}
		}
    }
	std::cout<<std::endl;
    return words_html;
}

Html_parser::Html_parser(unsigned int min_length, unsigned int max_length) : min_word_length(min_length), max_word_length(max_length)
{

}