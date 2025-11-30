//Задание 1. Паттерн «Команда»
#include <iostream>
#include <fstream>
#include <string>

// Класс-интерфейс для команды
class LogCommand {
public:
    virtual ~LogCommand() = default;
    virtual void print(const std::string& message) = 0;
};

// Логика записи
class LogReciever
{
    public:
        LogReciever() {};
        void write_console(const std::string& message)
        {
            std::cout << message << std::endl;
        }
        void save_file(const std::string& message,const std::string& path)
        {
            std::ofstream logFile(path);
            if (logFile.is_open()) {
                logFile << message << std::endl;
                logFile.close();
            }
            else {
                std::cout << "Не удалось открыть файл для записи." << std::endl;
            }
        }
};

// Команда на запись в консоль
class LogConsoleCommand: public LogCommand
{
    LogReciever& __logreciever;
    public:
        LogConsoleCommand(LogReciever& logreciever) : __logreciever{ logreciever } {};
        void print(const std::string& message) override
        {
            __logreciever.write_console(message);
        }
};

// Команда на запись в файл
class LogFileCommand: public LogCommand
{
    std::string __path{};
    LogReciever& __logreciever;
    public:
        LogFileCommand(LogReciever& logreciever, std::string path) : __logreciever{ logreciever }, __path{ path } {};
        void print(const std::string& message) override
        {
            __logreciever.save_file(message,__path);
        }
};


// Функция для выполнения команды
void print(LogCommand& command,const std::string& message)
{
    command.print(message);
}

int main()
{
    setlocale(LC_ALL,"Rus");
    LogReciever log;
    LogConsoleCommand cmd_console(log);
    LogFileCommand cmd_file(log, "application.log");
    
    print(cmd_console,"Запись в консоль");
    print(cmd_file, "Запись в файл");
    return EXIT_SUCCESS;
}