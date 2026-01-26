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
		void connect();
		void CreateDBTable();
};

#endif // DATA_BASE
