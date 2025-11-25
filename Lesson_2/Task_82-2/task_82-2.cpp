#include <iostream>
#include <string>
#include <map>
#include <memory>

class VeryHeavyDatabase
{
	public:
		virtual std::string get_data(const std::string& key)
		{
			return "Result of query: " + key;
		}
};

class CacheDB :public VeryHeavyDatabase
{
	std::map<std::string, std::string> __cache;
	std::unique_ptr<VeryHeavyDatabase> __real_db;

	public:
		CacheDB(std::unique_ptr<VeryHeavyDatabase> real_db) : __real_db(std::move(real_db)) {}
		std::string get_data(const std::string& key) override
		{
			if (__cache.find(key) == __cache.end())
			{
				std::cout << "Get data from Original Data base" << std::endl;
				__cache[key] = __real_db->get_data(key);
			}
			else
			{
				std::cout << "Get data from Cache Data base" << std::endl;
			}
			return __cache.at(key);
		}
};

class TestDB :public VeryHeavyDatabase
{
	std::unique_ptr<VeryHeavyDatabase> __real_db;
	public:
		TestDB(std::unique_ptr<VeryHeavyDatabase> real_db) : __real_db(std::move(real_db)) {}
		std::string get_data(const std::string& key) override
		{
			return "Test data";
		}
};

class OneShotDB : public VeryHeavyDatabase
{
	std::unique_ptr<VeryHeavyDatabase> __real_db;
	size_t limit_shots{}, count{};
	public:
		OneShotDB(std::unique_ptr<VeryHeavyDatabase> real_db, size_t shots) : __real_db(std::move(real_db)), limit_shots(shots) {}
		std::string get_data(const std::string& key) 
		{
			++count;
			return count <= limit_shots ? __real_db->get_data(key) : "Error. Limit of data query.";

		}
};

int main()
{
	setlocale(LC_ALL,"Russian");
	auto original_db = std::make_unique<VeryHeavyDatabase>();
	auto limit_db = std::make_unique<OneShotDB>(std::move(original_db), 4);
	std::cout << "\nРезультаты запросов в БД с лимитом обращений:" << std::endl;

	std::cout << limit_db->get_data("key1") << std::endl;
	std::cout << limit_db->get_data("key1") << std::endl;
	std::cout << limit_db->get_data("key1") << std::endl;
	std::cout << limit_db->get_data("key2") << std::endl;
	std::cout << limit_db->get_data("key2") << std::endl;
	std::cout << limit_db->get_data("key2") << std::endl;
	std::cout << limit_db->get_data("key2") << std::endl;
	
	return EXIT_SUCCESS;
}