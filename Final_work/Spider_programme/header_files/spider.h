#ifndef SPIDER
#define SPIDER

#include <iostream>
#include "thread_pool.h"
#include <openssl/opensslv.h>
#include "http_client_async_ssl.h"
#include "http_client_async.h"
#include "data_base.h"
#include "html_parser.h"
#include <memory>

struct Data
{
	std::string start_url;
	int	search_depth;
	int	min_word_length;
	int	max_word_length;
	int max_threads_num;
	std::string host;
	std::string port;
	std::string dbname;
	std::string user;
	std::string password;
	std::string db_connection;
};

class Spider
{
	private:
		std::string start_url;
		int	search_depth;
		
		int max_threads_num;
		Thread_pool tpool;
		std::vector<std::future<void>> results;
		std::mutex queue_mutex;

		std::string db_connection;
		Data_base* db;
		std::mutex mtx_db;

		Html_parser html_parser;

	public:
		Spider(Data ini_data);
		~Spider();
		Spider(const Spider& other) = delete;             // конструктор копирования
		Spider& operator=(const Spider& other) = delete;  // оператор присваивания
		Spider& operator=(Spider&& other) = delete;		  // оператор перемещающего присваивания

		void work();           // старт программы (добавление адреса в очередь, упаковка задачи)
		void task(std::string& url, int& url_depth);                            // выполнение задачи потоком
		void add_url_words_to_db(const std::string& url_str, std::map<std::string, unsigned int>& words);

};

#endif // SPIDER