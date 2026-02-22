#include "data_base.h"

Data_base::Data_base(const std::string& db_connection_string) : connection_str{ db_connection_string }
{
	
}

Data_base::~Data_base()
{
	if (conn) 
	{
		if (conn->is_open())
		{
			conn->close();
		}
		delete conn;
		std::cout << "\n Отключение БД!!!" << std::endl;
	}
}

// конструктор перемещения
Data_base::Data_base(Data_base&& other) noexcept 
{
	conn = other.conn; 
	other.conn = nullptr;
}

// оператор перемещающего присваивания
Data_base& Data_base::operator=(Data_base&& other) noexcept   
{
	conn = other.conn; 
	other.conn = nullptr;
	return *this;
}

void Data_base::connect()
{
	try
	{
		conn = new pqxx::connection(connection_str);
		std::cout << "\n Осуществлено подключение к базе данных поискового робота Spider!!!" << std::endl;
	}
	catch (const pqxx::broken_connection& e)
	{
		std::cerr << "Ошибка подключения: " << e.what() << std::endl;
	}
	catch (const std::exception& e)
	{
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
		std::cout << " Созданы таблицы документов и слов!!!" << std::endl << std::endl;
	}
	catch (pqxx::sql_error e)
	{
		std::cout << e.what() << std::endl;
		std::cout << "Ошибка создания таблиц БД!!!" << std::endl;
	}
}

bool Data_base::create_templates()
{
	if ((conn == nullptr) || (!(conn->is_open())))
	{
		std::cout << " Отсутствует подключение к БД" << std::endl;
		return false;
	}
	else
	{
		// Добавление данных в БД
		conn->prepare("add_url", "INSERT INTO DOCS(url) VALUES ($1) ON CONFLICT (url) DO UPDATE SET url = EXCLUDED.url RETURNING id");
		conn->prepare("add_word", "INSERT INTO WORDS(word) VALUES ($1) ON CONFLICT (word) DO UPDATE SET word = EXCLUDED.word RETURNING id");
		//conn->prepare("add_url_word", "INSERT INTO docs_words(id_url, id_word, quantity) VALUES($1, $2, 1) ON CONFLICT(id_url, id_word) DO "
		//	"UPDATE SET quantity = docs_words.quantity + 1 ");

		conn->prepare("add_url_word"," INSERT INTO docs_words(id_url, id_word, quantity) VALUES($1, $2, $3) "
			"ON CONFLICT (id_url, id_word) DO UPDATE SET quantity = docs_words.quantity + $3");

		conn->prepare("get_url_id","SELECT cl.id FROM DOCS cl WHERE cl.url=$1 ");
		conn->prepare("get_word_id", "SELECT cl.id FROM WORDS cl WHERE cl.word=$1 ");
	}
	return true;
}

// Метод добавление нового url в БД
int Data_base::add_new_url(const std::string& url_name)
{
	if (conn == nullptr)
	{
		std::cerr << "No database connection" << std::endl;
		return false;
	}
	
	try
	{
		pqxx::work tx(*conn);
		pqxx::result res = tx.exec_prepared("add_url", url_name);
		tx.commit();

		int new_id = res[0][0].as<int>();
		return new_id;
	}
	catch (const std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
		return false;
	}

}

// Метод добавление нового слова в БД
int Data_base::add_new_word(const std::string& word_name)
{
	if (conn == nullptr)
	{
		std::cerr << "No database connection" << std::endl;
		return false;
	}

	try
	{
		pqxx::work tx(*conn);
		pqxx::result res = tx.exec_prepared("add_word", word_name);
		tx.commit();

		int new_id = res[0][0].as<int>();
		return new_id;
	}
	catch (const std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
		return false;
	}
}

// Метод добавление url и слова в промежуточную таюлицу БД
void Data_base::add_new_url_word(int id_url,int id_word, int quantity)
{
	if (conn == nullptr)
	{
		std::cerr << "No database connection" << std::endl;
		return;
	}
	
	try
	{
		pqxx::work tx(*conn);

		pqxx::result res = tx.exec_prepared("add_url_word", id_url, id_word, quantity);
		tx.commit();
	}
	catch (const std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
		return;
	}
}

// Получение id_url из БД
int Data_base::get_id_url(const std::string& url_name)
{
	if (conn == nullptr)
	{
		std::cerr << "No database connection" << std::endl;
		return false;
	}

	try
	{
		pqxx::work tx(*conn);

		pqxx::result res = tx.exec_prepared("get_url_id", url_name);
		tx.commit();

		int id = res[0][0].as<int>();
		return id;
	}
	catch (const std::exception& ex)
	{
		//std::cout << ex.what() << std::endl;
		return -1;
	}
}

// Получение id_url из БД
int Data_base::get_id_word(const std::string& word_name)
{
	if (conn == nullptr)
	{
		std::cerr << "No database connection" << std::endl;
		return false;
	}

	try
	{
		pqxx::work tx(*conn);

		pqxx::result res = tx.exec_prepared("get_word_id", word_name);
		tx.commit();

		int id = res[0][0].as<int>();
		return id;
	}
	catch (const std::exception& ex)
	{
		//std::cout << ex.what() << std::endl;
		return -1;
	}

}

	