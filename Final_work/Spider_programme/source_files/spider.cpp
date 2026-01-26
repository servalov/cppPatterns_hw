#include "spider.h"

Spider::Spider(Data ini_data):tpool(ini_data.max_threads_num)
{
	// Инициализация переменных
	start_url = ini_data.start_url;
	search_depth = ini_data.search_depth;
	max_threads_num = ini_data.max_threads_num;
	db_connection = ini_data.db_connection;

	// Подключение к БД
	db = new Data_base(db_connection);
	db->connect();
	// Создание таблиц в БД
	db->CreateDBTable();
}

Spider::~Spider()
{
	delete db;
}

// Получение host и target
void Spider::get_host_target(std::string& new_url, std::string& host, std::string& target)
{

	int pos = new_url.find("/");
	target = new_url.substr(pos);
	host = new_url.erase(pos, target.length());
}

// Выполнение задачи потоком
void Spider::task(std::string& url, int& url_depth)
{
	// 1. Получение host и target страницы
	std::string host, target;
	get_host_target(url, host, target);
	
	std::cout << " Идет процесс выполнение задачи потоком " << std::this_thread::get_id() << std::endl;

	// 2. Запуск асинхронного считывания страницы с поддержкой ssl
	net::io_context ioc;
	ssl::context ctx{ ssl::context::tlsv12_client };
	ctx.load_verify_file("cacert.pem");

	std::string final_html;                          // код страницы
	auto handler = [&](std::string body) {
		final_html = std::move(body);                // сохранение результата
	};
	auto s = std::make_shared<session>(ioc, ctx, handler);
	s->run(host.c_str(), "443", target.c_str());
	ioc.run();

	// 3. Получение кода html страницы
	std::cout << " Код страницы получен! Длина: " << final_html.size() << std::endl;
	std::cout << "-----------------------------------------------------------------\n";
	std::cout << " Первые 100 символов строки: " << std::endl;
	std::cout << final_html.substr(0,100)<< std::endl;           
	std::cout << "-----------------------------------------------------------------\n";
	std::cout << " Задача выполнена. " << std::endl;


	// 4. Запуск html парсера (индексация страницы)
	// .....

	// 5. Добавление слов и ссылок в БД
	// ......

	// 6. Добавление новых ссылок в очередь с учетом глубины погружения
	// .......
	
	// Пример для отработки добавления новой ссылки в очередь (Добавить условие на запрет удаления незадействованных потоков при наличии работающих!!!)
	if (url_depth < 2)
	{
		std::string url_page = "trends.rbc.ru/trends/industry/60c85c599a7947f5776ad409";
		//url_page = "journal.topvisor.com/ru/dictionary/what-is-crawler/";
		//url_page = "neurohive.io/ru/osnovy-data-science/osnovy-nejronnyh-setej-algoritmy-obuchenie-funkcii-aktivacii-i-poteri/";
		type_task new_task([this](std::string& url, int& depth)
			{
				this->task(url, depth);
			});

		++url_depth;
		results.push_back(tpool.submit(std::move(new_task), url_page, url_depth));
	}
}

// Основная функция программы (добавление адреса в очередь, упаковка первой задачи, получение данных)
void Spider::work()
{
	int url_depth_start = 1;
	type_task first_task([this](std::string& url, int& depth)
		{
			this->task(url, depth);
		});

	results.push_back(tpool.submit(std::move(first_task), start_url, url_depth_start));

	for (auto& result : results)
	{
		result.get();            // завершения выполнения каждой задачи
	}
}




