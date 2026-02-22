#ifndef DATA_BASE
#define DATA_BASE

#include <pqxx/pqxx>
#include <iostream>

class Data_base
{
	private:
		const std::string connection_str;
		pqxx::connection* conn{nullptr};

	public:
		Data_base(const std::string& db_connection_string);
		~Data_base();
		Data_base(const Data_base& other) = delete;             //конструктор копирования
		Data_base& operator=(const Data_base& other) = delete;  //оператор присваивания
		Data_base& operator=(Data_base&& other) noexcept;       // оператор перемещающего присваивания
		Data_base(Data_base&& other) noexcept;	                // конструктор перемещения
		void connect();
		void CreateDBTable();
		bool create_templates();
		int add_new_url(const std::string& url_name);
		int add_new_word(const std::string& word_name);
		void add_new_url_word(int id_url, int id_word, int quantity);
		int get_id_url(const std::string& url_name);
		int get_id_word(const std::string& word_name);
};

#endif // DATA_BASE
