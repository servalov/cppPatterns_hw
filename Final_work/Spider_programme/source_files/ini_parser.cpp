#include "ini_parser.h"

// Функция на удаление пробелов в начале и конце строки
std::string erase_ispace(std::string& str)
{
	size_t pos_begin = str.find_first_not_of(' ');
	size_t pos_end = str.find_last_not_of(" \n");
	str = str.substr(pos_begin);
	str = str.substr(0, pos_end + 1);
	return str;
}

// Функция на проверку строки из пробелов
bool str_is_whitespace(const std::string& str)
{
	for (char c : str)
	{
		if (!std::isspace(c))
		{
			return false;
		}
	}
	return true;
}

ini_parcer::ini_parcer(std::string n_file) : name_file{ n_file }
{
	// Открываем файл для чтения и заносим данные в словарь
	std::ifstream file;
	file.open(name_file);

	std::string line{};
	std::string current_section{};

	if (file.is_open()) {
		while (std::getline(file, line))
		{
			if (line.empty() || line[0] == ';' || line[0] == ' ') continue;
			if (str_is_whitespace(line)) continue;

			if (line[0] == '[' && line.back() == ']')
			{
				current_section = line.substr(1, line.length() - 2);
				line = erase_ispace(line);
				config[current_section];
			}
			else
			{
				line = erase_ispace(line);
				size_t position = line.find('=');
				std::string key = line.substr(0, position);
				std::string value = line.substr(position + 1);
				key = erase_ispace(key);
				value = erase_ispace(value);

				size_t pos = line.find(';');
				value = value.substr(0, pos);

				config[current_section][key] = value;
			}
		}
		std::cout << " Загружен ini-файл " << name_file << " с настройками программы"<<std::endl;
		file.close();
	}
	else
	{
		std::cout << "ini-файл " << name_file <<" не загружен !!!"<<std::endl;
	}
	
}

ini_parcer::~ini_parcer()
{

}

