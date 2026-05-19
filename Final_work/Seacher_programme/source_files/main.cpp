#include <iostream>
#include "ini_parser.h"
#include "search.h"
#include <windows.h>
#include "http_server_async.h"

int main()
{
    
   setlocale(LC_ALL, "Russian");
   SetConsoleOutputCP(1251);
   try
   {

       std::cout << "Программа-поисковик (HTTP-сервер)." << std::endl;
       std::cout << "-----------------------------------------" << std::endl;


       ini_parcer parser("config_ini_searcher.txt");         // файл с начальными данными для программы
       Data ini_data;

       ini_data.server_host = parser.get_value<std::string>("Server", "host");
       ini_data.server_port = parser.get_value<std::string>("Server", "port");
       ini_data.host = parser.get_value<std::string>("Database", "host");
       ini_data.port = parser.get_value<std::string>("Database", "port");
       ini_data.dbname = parser.get_value<std::string>("Database", "dbname");
       ini_data.user = parser.get_value<std::string>("Database", "user");
       ini_data.password = parser.get_value<std::string>("Database", "password");
       ini_data.search_results = parser.get_value<int>("Search_settings", "search_results");
       ini_data.min_word_length = parser.get_value<int>("Search_settings", "min_word_length");
       ini_data.max_word_length = parser.get_value<int>("Search_settings", "max_word_length");

       ini_data.db_connection = "host= " + ini_data.host + " port= " + ini_data.port + " dbname = " + ini_data.dbname + " user = " + ini_data.user + " password = " + ini_data.password;


       std::cout << "\n Информация по настройкам программы Searcher:" << std::endl;
       std::cout << " server_host = " << ini_data.server_host << std::endl;
       std::cout << " server_port = " << ini_data.server_port << std::endl;

       std::cout << "\n Информация по настройкам БД" << std::endl;
       std::cout << " host = " << ini_data.host << std::endl;
       std::cout << " port = " << ini_data.port << std::endl;
       std::cout << " dbname = " << ini_data.dbname << std::endl;
       std::cout << " user = " << ini_data.user << std::endl;
       std::cout << " password = " << ini_data.password << std::endl;

       std::cout << "\n Информация по настройкам HTTP-сервер" << std::endl;
       std::cout << " Search_results = " << ini_data.search_results << std::endl;
       std::cout << " Min_word_length = " << ini_data.min_word_length << std::endl;
       std::cout << " Max_word_length = " << ini_data.max_word_length << std::endl;

       std::cout << "\n Начало работы поисковика ..." << std::endl;
       
       Search search(ini_data);

       std::cout << " Для начала работы поисковика нажмите Enter...";
       std::cin.get();

       search.work();
   }

    catch (const std::exception& e)
    {
        std::cerr << "\n   Ошибка: " << e.what() << std::endl;
        std::cerr << "     Программа нештатно завершила работу!!!" << std::endl;
        return 1;
    }

	return EXIT_SUCCESS;
}