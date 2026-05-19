#include "spider.h"
#include "html_parser.h"

// Функция кодировки с utf8_to_cp1251
std::string utf8_to_cp1251_spider(std::string const& utf8)
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

Spider::Spider(Data ini_data):tpool(ini_data.max_threads_num), html_parser(ini_data.min_word_length, ini_data.max_word_length)
{
	// Инициализация переменных
	start_url = ini_data.start_url;
	search_depth = ini_data.search_depth;
	max_threads_num = ini_data.max_threads_num;
	db_connection = ini_data.db_connection;
	
	// Подключение к БД
	//db = new Data_base(db_connection);

	try
	{
		db = std::make_unique<Data_base>(db_connection);
		db->connect();
		// Создание таблиц в БД
		db->CreateDBTable();
		// Создание шаблонов
		db->create_templates();
	}
	catch (const std::exception& e)
	{
		std::cerr << "\n   Ошибка: " << e.what() << std::endl;
		std::cerr << "     Поисковый робот Spider не может начать работу без базы данных!!!" << std::endl;
	}

}

Spider::~Spider()
{
	std::cout << "\n -------------- Завершение работы поискового робота Spider ----------" << std::endl;
	
	for (auto& res : results)
	{
		if (res.valid())
		{
			res.wait();            // ждем завершения потока
		}
	}
	results.clear();

	std::cout << "\n Завершение работы с базой данных поискового робота Spider" << std::endl;
	/*
	if (db)
	{
		delete db;
		db = nullptr;
	}
	*/
}

// Выполнение задачи потоком
void Spider::task(std::string& url, int url_depth)
{
	std::this_thread::sleep_for(std::chrono::microseconds(100));
	
	// 1. Проверка на существующие страницы в БД
	if (db->get_id_url(url) > 0)
	{
		std::cout << " Такая страница имеется в БД и ранее была обработана." << std::endl;
		return;
	}
	
	// 2. Получение host и target страницы
	std::string host, target;
	html_parser.get_host_target(url, host, target);
	
	// 3. Оборачиваем возвращаемый результат в лямбду
	std::string final_html;
	int http_status_code{ 0 };     // код страницы
	auto handler = [&](std::string body, int status_code)
	{
		final_html = std::move(body);                // сохранение результата
		http_status_code = status_code;
	};
	
	// 4. Определяем соединение по протоколу
	std::cout<<" \n\n "<<++count_task<< ". Поток " << std::this_thread::get_id() << " начал выполнять задачу. Работа с url = "<< url << std::endl;
	std::cout << "      Число активных задач составляет " << active_tasks << std::endl << std::endl;

	net::io_context ioc;
	//ssl::context ctx{ ssl::context::tlsv12_client };
	auto ctx = std::make_shared<ssl::context>(ssl::context::tls_client);
	ctx->load_verify_file("cacert.pem");
	
	if (url.find("https://") == 0)
	{
		// Запуск асинхронного считывания страницы с поддержкой ssl (для https://)
		auto s = std::make_shared<session_ssl>(ioc, ctx, handler);
		s->run_ssl(host.c_str(), "443", target.c_str());
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

	// 5. Результаты загрузки url
	if (final_html.size() == 0)
	{
		std::unique_lock<std::mutex> lk(queue_mutex);
		std::cout<<"\n     Поток " << std::this_thread::get_id()<< ". Отказ выполнения запроса. Статус: Error "<< http_status_code<<". Пропуск запрашиваемого url адреса." << std::endl;
		lk.unlock();
		return;
	}
	
	// 6. Получение новых ссылок
	std::vector<std::string> page_links = html_parser.get_all_links(final_html, url);
	/*
	for (const auto& url : page_links)
	{
		std::cout << url << std::endl;
	}
	*/

	// 7. Индексация страницы и получение слов для добавления их в БД
	final_html = html_parser.html_index(final_html);
	/*
	// Запись ответа (HTML) в файл
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

	// 8. Поиск слов и добавление их в БД
	std::map<std::string, unsigned int> words = html_parser.new_words(final_html,url);
	
	// 9. Работа с БД. Добавление новых ссылок и слов.
	std::unique_lock<std::mutex> lk_db(mtx_db);
	
	std::cout << "\n    Поток " << std::this_thread::get_id() <<" завершил работу: "<< std::endl;
	std::cout << "    Загружены данные с запрашиваемого url." << std::endl;
	std::cout << "    Длина строки html " << final_html.size() << " символов. " << std::endl;
	std::cout << "    На странице найдено " << page_links.size() << " url-адресов." << std::endl;
	std::cout << "    Произведена индексация html строки." << std::endl;
	
	if (words.size()>0)
	{
		std::cout << "\n    Найдены следующие слова в html-строке url: " << url << "\n" << std::endl << "       ";
		int count{};
		size_t str_len{};
		std::string word_str{};
		for (const auto& word : words)
		{
			word_str = utf8_to_cp1251_spider(word.first);
			str_len = word_str.size();
			std::cout << word_str << " (длина: " << str_len << ") ; ";
			++count;
			if (count % 5 == 0)
			{
				std::cout << std::endl << "       ";
			}
		}
		std::cout << std::endl;

		add_url_words_to_db(url, words);
		if ((url_depth < search_depth) && (page_links.size() > 0))
		{
			std::cout << "\n ------> В очередь для thread_pool (пул потоков) добавлено " << page_links.size() << " новых задач. " << std::endl;
		}
	}
	else
	{
		std::cout << "\n    В html-строке url не найдены слова. "<<std::endl;
	}
	
	std::cout << std::endl;

	lk_db.unlock();

	// 10. Добавление новых ссылок в очередь с учетом глубины погружения
	if (url_depth < search_depth)
	{
		for (auto& url : page_links)
		{
			active_tasks++;    // увеличение счетчика задач

			type_task new_task([this](std::string& url, int& depth)
				{
					this->task(url, depth);
					this->active_tasks--;    // задача завершена
					//std::cout << "     Число активных задач составляет " << active_tasks << std::endl;
				});

			std::unique_lock<std::mutex> lk_queue(queue_mutex);      // зашита result
			results.push_back(tpool.submit(std::move(new_task), url, url_depth+1));
			lk_queue.unlock();
		}
	}
}

// Основная функция программы (добавление адреса в очередь, упаковка первой задачи, получение данных)
void Spider::work()
{
	std::cout << "\n -------------- Старт работы поискового робота Spider --------------" << std::endl;
	std::cout << "\n Стартовая страница : start_url = "<< start_url << std::endl;

	int url_depth_start = 1;
	active_tasks++;           // добавление первой задачи
	
	type_task first_task([this](std::string& url, int& depth)
		{
			this->task(url, depth);
			this->active_tasks--;    // задача завершена
		});

	std::unique_lock<std::mutex> lk_queue(queue_mutex);   // захват mutex для защиты доступа к вектору
	results.push_back(tpool.submit(std::move(first_task), start_url, url_depth_start));
	lk_queue.unlock();

	// Цикл для завершения всех задач перед (work не закончится, пока последний поток не закончит обратотку)
	while (active_tasks > 0) 
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	for (auto& result : results)
	{
		if(result.valid()) result.get();          
	}
}

//Добавление url и новых слов в БД
void Spider::add_url_words_to_db(const std::string& url_str, std::map<std::string, unsigned int>& words)
{
	if (words.empty()) return;
	if (!db) return;


	int url_word, url_id, count_word{}, count_url_word{};
	url_id = db->add_new_url(url_str);
	if (url_id == -1) return;

	std::cout << "\n    Результаты обработки url = " << url_str << " следующие:" << std::endl;
	std::cout << "       В таблицу c DOCS (документы) БД добавлен новый url-адрес: " << url_str << std::endl;
	
	for (const auto& word : words)
	{
		// Поиск слова в БД
		url_word = db->get_id_word(word.first);
		if (url_word < 0)        // если такого слова нет в таблице words
		{
			url_word = db->add_new_word(word.first);               // добавление в таблицу words
			db->add_new_url_word(url_id, url_word, word.second);   // добавление в промежуточную таблицу
			++count_word;
			++count_url_word;
		}
		else   // если такое слово есть в таблице words
		{
			db->add_new_url_word(url_id, url_word, word.second);   // добавление в промежуточную таблицу
			++count_url_word;
		}
	}
	
	if (count_url_word>0)
	{
		std::cout << "       В промежуточную таблицу docs_words БД добавлено " << count_url_word << " записей. "<<std::endl;
	}
	
	if (count_word > 0)
	{
		std::cout << "       В таблицу c WORDS (слова поиска) БД добавлены " << count_word << " новых слов. " << std::endl;
	}

}