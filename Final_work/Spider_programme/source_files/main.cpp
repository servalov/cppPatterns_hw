#include <iostream>
#include "ini_parser.h"
#include "spider.h"

int main()
{
	setlocale(LC_ALL, "Russian");
	SetConsoleOutputCP(1251);
	
	std::cout << "Поисковый робот Spider (HTTP-клиент)." << std::endl;
	std::cout << "-----------------------------------------" << std::endl;

	ini_parcer parser("config_ini_spider.txt");         // файл с начальными данными для программы
	
	Data ini_data;
	ini_data.start_url = parser.get_value<std::string>("Page","start_page");
	ini_data.search_depth = parser.get_value<int>("Search_settings", "search_depth");
	ini_data.min_word_length = parser.get_value<int>("Search_settings", "min_word_length");
	ini_data.max_word_length = parser.get_value<int>("Search_settings", "max_word_length");
	ini_data.max_threads_num = parser.get_value<int>("Process_settings", "max_threads");
	ini_data.host= parser.get_value<std::string>("Database", "host");
	ini_data.port = parser.get_value<std::string>("Database", "port");
	ini_data.dbname = parser.get_value<std::string>("Database", "dbname");
	ini_data.user = parser.get_value<std::string>("Database", "user");
	ini_data.password = parser.get_value<std::string>("Database", "password");

	size_t numTread{std::thread::hardware_concurrency()};
	ini_data.max_threads_num = ini_data.max_threads_num > numTread ? numTread : ini_data.max_threads_num;

	ini_data.db_connection = "host= " + ini_data.host + " port= " + ini_data.port + " dbname = " + ini_data.dbname+ " user = " + ini_data.user + " password = "+ ini_data.password;

	std::cout << "\n Информация по настройкам программы Spider:" << std::endl;
	std::cout << " Start url = " << ini_data.start_url<< std::endl;
	std::cout << " Max_threads_num = " << ini_data.max_threads_num << std::endl;

	std::cout << "\n Информация по настройкам HTML парсера" << std::endl;
	std::cout << " Min_word_length = " << ini_data.min_word_length << std::endl;
	std::cout << " Max_word_length = " << ini_data.max_word_length << std::endl;
	std::cout << " Search_depth = " << ini_data.search_depth << std::endl;

	std::cout << "\n Информация по настройкам БД" << std::endl;
	std::cout << " host = " << ini_data.host << std::endl;
	std::cout << " port = " << ini_data.port << std::endl;
	std::cout << " dbname = " << ini_data.dbname << std::endl;
	std::cout << " user = " << ini_data.user << std::endl;
	std::cout << " password = " << ini_data.password << std::endl;

	Spider spider(ini_data);
	spider.work();

	return EXIT_SUCCESS;
}