#ifndef INI_PARSER
#define INI_PARSER

#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>

// Класс ini_parcer для работы с ini файлами
class ini_parcer
{
private:
	std::map<std::string, std::map<std::string, std::string>> config;
	std::string name_file{};

public:

	ini_parcer(std::string n_file);    // Конструктор
	~ini_parcer();                     // Деструктор
	ini_parcer(std::string& n_file) = delete;                     // Конструктор копирования
	ini_parcer& operator=(const std::string& a_other) = delete;   // Оператор присваивания
	ini_parcer(ini_parcer&& other) noexcept;                      // конструктор перемещения
	ini_parcer& operator=(ini_parcer&& other) = delete;		      // оператор перемещающего присваивания

	// Шаблонный метод для получения данных из ini файла
	template <typename T>
	T get_value(std::string section, std::string par)
	{

		// Проверяем есть ли данные в словаре
		for (const auto& section_pair : config)
		{
			if (section_pair.first == section)
			{
				for (const auto& key_value_pair : section_pair.second)
				{
					if (key_value_pair.first == par)
					{
						std::stringstream ss(key_value_pair.second);
						T value;
						ss >> value;
						if (ss.fail()) throw std::runtime_error("В файле нет значения для переменной данного типа !!!");
						return value;
					}
				}
			}
		}
		throw std::runtime_error("В файле нет значения для этой переменной!!!");
		T value_empty{};
		return  value_empty;
	}
};

#endif // INI_PARSER
