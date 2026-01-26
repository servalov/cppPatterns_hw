#include "data_base.h"

Data_base::Data_base(const std::string& db_connection_string) : connection_str{ db_connection_string }
{
	
}

Data_base::~Data_base()
{
	delete conn;
	std::cout << "\n Отключение БД!!!" << std::endl;
}

void Data_base::connect()
{
	try
	{
		conn = new pqxx::connection(connection_str);
		std::cout << "\n Осуществлено подключение к базе данных поискового робота Spider!!!" << std::endl;
	}
	catch (const pqxx::broken_connection& e) {
		std::cerr << "Ошибка подключения: " << e.what() << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Другая ошибка: " << e.what() << std::endl;
	}
}

void Data_base::CreateDBTable()
{
	try
	{
		pqxx::work tx(*conn);

		//таблица документов
		tx.exec(
			"CREATE TABLE IF NOT EXISTS DOCS ("
			"id SERIAL PRIMARY KEY,"
			"url VARCHAR(300) NOT NULL UNIQUE"
			")"
		);
		
		//таблица слов
		tx.exec(
			"CREATE TABLE IF NOT EXISTS WORLDS ( "
			"id SERIAL PRIMARY KEY,"
			"word VARCHAR(40) NOT NULL UNIQUE"
			")"
		);

		//промежуточная таблица	
		tx.exec(
			"CREATE TABLE IF NOT EXISTS docs_words ( "
			"id_url INTEGER REFERENCES DOCS(id), "
			"id_word INTEGER REFERENCES WORLDS(id), "
			"quantity INTEGER NOT NULL,"
			"CONSTRAINT pk PRIMARY KEY(id_url,id_word)"
			")"
		);
		
		tx.commit();
		std::cout << " Созданы таблицы документов и слов!!!" << std::endl;
	}
	catch (pqxx::sql_error e)
	{
		std::cout << e.what() << std::endl;
		std::cout << "Ошибка создания таблиц БД!!!" << std::endl;
	}
}

	