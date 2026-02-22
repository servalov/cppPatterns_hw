#ifndef HTML_PARSER
#define HTML_PARSER

#include <string>
#include <regex>
#include <map>
#include <sstream>
#include <iostream>
#include <boost/locale.hpp>
#include <windows.h>

// Класс для парсера кода HTML страницы
class Html_parser
{
private:
	unsigned int min_word_length{};
	unsigned int max_word_length{};

public:
	Html_parser(unsigned int min_length, unsigned int max_length);
	std::string get_base_path(const std::string& in_str);                                                   // Функция получения path, где лежит страница
	std::string complete_url(const std::string& in_url, const std::string& url_base);                       // Функция получения полного url
	std::vector<std::string> get_all_links(const std::string& html_code, const std::string& current_url);   // Получение ссылок текущей страницы
	void get_host_target(std::string& new_url, std::string& host, std::string& target);
	std::string html_index(const std::string& html_str);   		                                            // Индексирование страницы
	std::map<std::string, unsigned int> new_words(const std::string& html_str, const std::string& current_url);
	
};

#endif // HTML_PARSER
