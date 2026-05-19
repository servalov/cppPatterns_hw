#include "search.h"
#include <vector>

Search::Search(Data ini_data)
{
	search_results = ini_data.search_results;
	min_word_length = ini_data.min_word_length;
	max_word_length = ini_data.max_word_length;
	db_connection = ini_data.db_connection;

	server_host = ini_data.server_host;
	server_port = static_cast<unsigned short>(std::stoi(ini_data.server_port));

	try
	{
		// Подключение к БД
		db = new Data_base(db_connection);
		db->connect();
		// Создание таблиц в БД
		db->CreateDBTable();
	}
	catch (const std::exception& e)
	{
		std::cerr << "\n   Ошибка: " << e.what() << std::endl;
		std::cerr << "     Поисковик не может начать работу без базы данных!!!" << std::endl;
	}
}

Search::~Search()
{
	
	if (db)
	{
		delete db;
		db = nullptr;
	}
	
	std::cout << " Удаление объекта Search" << std::endl;
}

void Search::work()
{

	std::cout << "\n -------------- Старт работы поисковика --------------" << std::endl;

	auto const threads = std::max<std::size_t>(1, std::thread::hardware_concurrency());
	net::io_context ioc{ static_cast<int>(threads-2) };

	auto endpoint = tcp::endpoint{ net::ip::make_address(server_host), server_port };
	std::make_shared<listener>(ioc, endpoint, search_results, min_word_length, max_word_length, db)->run();

	std::vector<std::thread> v;
	v.reserve(threads - 1);
	for (auto i = threads - 1; i > 0; --i)
		v.emplace_back(
			[&ioc]
			{
				ioc.run();
			});

	std::cout << "HTTP_Server старт ..." << std::endl;
	ioc.run();

	for (auto& t : v)
	{
		if (t.joinable()) {
			t.join();
		}
	}

}