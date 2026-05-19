#ifndef SEARCH
#define SEARCH

#include <iostream>
#include "http_server_async.h"
#include "data_base.h"

struct Data
{
	std::string server_host;
	std::string server_port;
	std::string host;
	std::string port;
	std::string dbname;
	std::string user;
	std::string password;
	std::string db_connection;
	int	search_results;
	int min_word_length;
	int max_word_length;
};

class Search
{
private:

	std::string server_host;
	unsigned short server_port;
	std::string db_connection;
	int	search_results;
	int min_word_length;
	int max_word_length;

	Data_base* db{ nullptr };

public:
	Search(Data ini_data);
	~Search();
	Search(const Search& other) = delete;             // конструктор копирования
	Search& operator=(const Search& other) = delete;  // оператор присваивания
	Search& operator=(Search&& other) = delete;		  // оператор перемещающего присваивания

	void work();           // старт программы (добавление адреса в очередь, упаковка задачи)

};

#endif // !SEARCH