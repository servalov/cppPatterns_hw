#include "search.h"
#include <vector>

Search::Search(Data ini_data)
{
	search_results = ini_data.search_results;
	db_connection = ini_data.db_connection;

	server_host = ini_data.server_host;
	server_port = static_cast<unsigned short>(std::stoi(ini_data.server_port));

	// Подключение к БД
	db = new Data_base(db_connection);
	db->connect();
	// Создание таблиц в БД
	db->CreateDBTable();
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

	auto const threads = 1;
	net::io_context ioc{ threads };

	auto endpoint = tcp::endpoint{ net::ip::make_address(server_host), server_port };
	std::make_shared<listener>(ioc, endpoint)->run();

	std::vector<std::thread> v;
	v.reserve(threads - 1);
	for (auto i = threads - 1; i > 0; --i)
		v.emplace_back(
			[&ioc]
			{
				ioc.run();
			});

	std::cout << " HTTP_Server старт ..." << std::endl;
	ioc.run();

	
}