#include "data_base.h"

// Конструктор
Data_base::Data_base(const std::string& db_connection_string) : connection_str{ db_connection_string }
{

}

// Деструктор
Data_base::~Data_base()
{
	if (conn)
	{
		if (conn->is_open())
		{
			conn->close();
		}
		delete conn;
		std::cout << "\n Завершение работы с БД." << std::endl;
	}
}

// Конструктор перемещения
Data_base::Data_base(Data_base&& other) noexcept
{
	conn = other.conn;
	other.conn = nullptr;
}

// оператор перемещающего присваивания
Data_base& Data_base::operator=(Data_base&& other) noexcept
{
	if (this != &other)
	{
		delete conn; // Защита от утечки памяти старого соединения
		conn = other.conn;
		other.conn = nullptr;
	}
	return *this;
}

void Data_base::connect()
{
	try
	{
		if (conn)
		{
			delete conn;
		}

		conn = new pqxx::connection(connection_str);
		std::cout << " 1. Осуществлено подключение к базе данных " << std::endl;
	}
	catch (const pqxx::broken_connection& e)
	{
		std::cerr << "Ошибка подключения базе данных " << e.what() << std::endl;
		conn = nullptr;
		throw;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Другая ошибка: " << e.what() << std::endl;
		conn = nullptr;
		throw;
	}
}

void Data_base::CreateDBTable()
{
	if ((conn == nullptr) || (!(conn->is_open())))
	{
		std::cout << "\n Ошибка: создание таблиц без подключения к БД. " << std::endl;
		return;
	}
	
	try
	{
		pqxx::work tx(*conn);

		//таблица документов
		tx.exec(
			"CREATE TABLE IF NOT EXISTS DOCS ("
			"id SERIAL PRIMARY KEY,"
			"url VARCHAR(1000) NOT NULL UNIQUE"
			")"
		);

		//таблица слов
		tx.exec(
			"CREATE TABLE IF NOT EXISTS WORDS ( "
			"id SERIAL PRIMARY KEY,"
			"word VARCHAR(40) NOT NULL UNIQUE"
			")"
		);

		//промежуточная таблица	
		tx.exec(
			"CREATE TABLE IF NOT EXISTS docs_words ( "
			"id_url INTEGER REFERENCES DOCS(id), "
			"id_word INTEGER REFERENCES WORDS(id), "
			"quantity INTEGER NOT NULL,"
			"CONSTRAINT pk PRIMARY KEY(id_url,id_word)"
			")"
		);

		tx.commit();
		std::cout << " 2. Подготовлены таблицы документов и слов БД." << std::endl << std::endl;
	}
	catch (pqxx::sql_error e)
	{
		std::cout << e.what() << std::endl;
		std::cout << "Ошибка подготовки и создания таблиц БД!!!" << std::endl;
		return;
	}
}

// Подготовка строки из запрашиваемых слов
std::string form_words_where_in(const std::set<std::string>& words, pqxx::work& tx)
{
	if (words.empty())
	{
		return "false";
	}
	
	std::string list;
	try 
	{
		for (const auto& w : words)
		{
			if (!list.empty())
			{
				list += ", ";
			}
			list += tx.quote(w);
		}
	}
	catch (const pqxx::argument_error& e)
	{
		std::cerr << "Ошибка подготовки строки: " << e.what() << std::endl;
	}
		
	return "word IN("+list+")";
}

std::string Data_base::poisk_url_by_words(const std::set<std::string>& search_words, int search_results)
{
	
	if (conn == nullptr || !conn->is_open())
	{
		std::cerr << " Отсутствует подключение к БД!!!" << std::endl;
		return "<p>Ошибка: база данных недоступна.</p>";
	}
	
	std::cout << " Вызов функции получения списка адресов по искомым словам из запроса пользователя" << std::endl;
	std::unordered_map<std::string, int> result_url_list;
	std::string search_res_str;

	if (search_words.empty())
	{
		return " ";
	}

	try
	{
		pqxx::work tx(*conn);
		std::string where_in = form_words_where_in(search_words, tx);
		//std::cout << " where_in = " << where_in << std::endl;

		std::string request_str = "select d.url, sum (dw.quantity) as rank "
			"from DOCS d "
			"join docs_words dw on d.id=dw.id_url "
			"join WORDS w on dw.id_word=w.id "
			"where " + where_in +
			"group by d.url "
			"order by rank desc "
			"limit " + std::to_string(search_results) + " ;";

		auto query_res = tx.exec(request_str);

		for (auto row : query_res)
		{
			result_url_list[row["url"].as<std::string>()] = row["rank"].as<int>();
		}

		tx.commit();

		std::cout << "\nРезультат запроса (отсортированы): " << std::endl;
		int i = 1;
		for (auto row : query_res)
		{
			std::string url = row["url"].as<std::string>();
			int rank = row["rank"].as<int>();
			std::cout << " URL: " << url << " | Частота слов : " << rank << std::endl;
			search_res_str += " <p> " + std::to_string(i) + ". <a href=\"" + url + "\">" + url + "</a>  (частота слов: " + std::to_string(rank) + ") </p>\n";
			++i;
		}

		if (search_res_str.empty())
		{
			search_res_str = "<p> Нет результатов на ваш запрос. Попробуйте еще раз.</p>\n";
		}

	}
	catch (pqxx::sql_error e)
	{
		std::cout << e.what() << std::endl;
		std::cout << "Ошибка создания запроса поиска адресов страниц!!!" << std::endl;

		std::cerr << "SQL Error: " << e.what() << std::endl;
		std::cerr << "Query: " << e.query() << std::endl;
		return "<p>Ошибка базы данных при поиске.</p>";
	}
	return search_res_str;
}