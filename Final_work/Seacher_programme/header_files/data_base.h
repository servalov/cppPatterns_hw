#ifndef DATA_BASE
#define DATA_BASE

#include <pqxx/pqxx>
#include <iostream>
#include <set>
#include <unordered_map>

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
	std::string poisk_url_by_words(const std::set<std::string>& search_words, int search_results);
};

#endif // DATA_BASE