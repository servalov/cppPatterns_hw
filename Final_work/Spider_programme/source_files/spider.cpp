#include "spider.h"
#include "html_parser.h"

Spider::Spider(Data ini_data):tpool(ini_data.max_threads_num), html_parser(ini_data.min_word_length, ini_data.max_word_length)
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
	// Создание шаблонов
	db->create_templates();
}

Spider::~Spider()
{
	for (auto& res : results)
	{
		if (res.valid())
		{
			res.wait();            // ждем завершения потока
		}
	}
	results.clear();
	
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	if (db)
	{
		delete db;
		db = nullptr;
	}
}

// Выполнение задачи потоком
void Spider::task(std::string& url, int& url_depth)
{
	//url = "https://trends.rbc.ru/trends/industry/60c85c599a7947f5776ad409";   для проверки
	// 1. Получение host и target страницы
	std::string host, target;
	html_parser.get_host_target(url, host, target);
	
	// 2. Оборачиваем возвращаемый результат в лямбду
	std::string final_html;                          // код страницы
	auto handler = [&](std::string body) {
		final_html = std::move(body);                // сохранение результата
	};
	
	// 3. Определяем соединение по протоколу
	std::cout << " \n Идет выполнение задачи потоком " << std::this_thread::get_id() << std::endl;

	net::io_context ioc;
	//ssl::context ctx{ ssl::context::tlsv12_client };
	auto ctx = std::make_shared<ssl::context>(ssl::context::tls_client);
	ctx->load_verify_file("cacert.pem");
	
	if (url.find("https://") == 0)
	{
		// Запуск асинхронного считывания страницы с поддержкой ssl (для https://)
		auto s = std::make_shared<session_ssl>(ioc, ctx, handler);
		s->run(host.c_str(), "443", target.c_str());
		ioc.run();

	}
	else if (url.find("http://") == 0)
	{
		// Запуск асинхронного считывания страницы (для http://)
		auto s = std::make_shared<session>(ioc, ctx, handler);
		s->run(host.c_str(), "80", target.c_str());
		ioc.run();
	}
	else return;

	// 4. Получение новых ссылок
	if (final_html.size() == 0)
	{
		std::cout << "\n Пропуск страницы. Error of 4xx !!!" << std::endl;
		return;
	}
	else
	{
		std::cout << "\n Получены данные страницы. Длина html " << final_html.size() <<" символов "<< std::endl;
	}
	
	// 4. Получение новых ссылок
	std::vector<std::string> page_links = html_parser.get_all_links(final_html, url);
	/*
		for (const auto& url : page_links)
		{
			std::cout << url << std::endl;
		}
	*/

	// 5. Индексация страницы и получение слов для добавления их в БД
	final_html = html_parser.html_index(final_html);
	std::map<std::string, unsigned int> words = html_parser.new_words(final_html,url);
	add_url_words_to_db(url, words);

	// Запись ответа (HTML) в файл
	/*
	std::ofstream outFile("result_html.txt");
	if (outFile.is_open()) {
		outFile << final_html;
		outFile.close();
		//std::cout << "HTML-код страницы сохранен в result_html.txt" << std::endl;
	}
	else {
		std::cerr << "Не удалось открыть файл для записи." << std::endl;
	}
	*/

	// 6. Добавление новых ссылок в очередь с учетом глубины погружения
	if (url_depth < search_depth)
	{
		
		std::cout << " Получено " << page_links.size() << " новых ссылок!!!" << std::endl;
		std::unique_lock<std::mutex> lk_queue(queue_mutex);
		for (auto& url : page_links)
		{
			type_task new_task([this](std::string& url, int& depth)
				{
					this->task(url, depth);
				});

			++url_depth;
			results.push_back(tpool.submit(std::move(new_task), url, url_depth));
		}
		
		//std::cout << " Получено " << page_links.size() <<" новых ссылок "<< std::endl;
		std::cout << " В очередь добавлено " << page_links.size() <<" задачи!!! "<< std::endl;
		
		lk_queue.unlock();
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

//Добавление url и новых слов в БД
void Spider::add_url_words_to_db(const std::string& url_str, std::map<std::string, unsigned int>& words)
{
	std::unique_lock<std::mutex> lk_db(mtx_db);
	
	if (words.empty()) return;

	if (db->get_id_url(url_str) > 0)
	{
		// есть такая страница в БД 
		return;
	}

	int url_word, url_id, count_word{};
	url_id = db->add_new_url(url_str);
	std::cout << " В БД добавлена новая ссылка: " << url_str << std::endl;
	for (const auto& word : words)
	{
		url_word = db->get_id_word(word.first);
		if (url_word < 0)
		{
			url_word = db->add_new_word(word.first);
			db->add_new_url_word(url_id, url_word, word.second);
			++count_word;
		}
		else
		{
			db->add_new_url_word(url_id, url_word, word.second);
		}
	}
	if (count_word > 0) std::cout << " В БД добавлено " << count_word << " новых слов !!!" << std::endl;
	lk_db.unlock();
}