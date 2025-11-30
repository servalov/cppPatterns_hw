//Задание 2. Паттерн «Наблюдатель»
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

class Observer
{
    public:
        virtual void onWarning(const std::string& message) {}
        virtual void onError(const std::string& message) {}
        virtual void onFatalError(const std::string& message) {}
};

class LogObserver
{
    private:
        std::vector<Observer*> __observers;
    public:
        void warning(const std::string& message) const
        {
            for (Observer* obs : __observers)
            {
                obs->onWarning(message);
            }
        }
        void error(const std::string& message) const
        {
            for (Observer* obs : __observers)
            {
                obs->onError(message);
            }
        }
        void fatalError(const std::string& message) const
        {
            for (Observer* obs : __observers)
            {
                obs->onFatalError(message);
            }
        }
        void add_observer(Observer* observer)
        {
            __observers.push_back(observer);
        }
        void remove_observer(Observer* observer)
        {
            auto it = remove(__observers.begin(), __observers.end(), observer);
            __observers.erase(it, __observers.end());
        }
};

class WarningObserver : public Observer
{
    public:
        void onWarning(const std::string& message) override
        {
            std::cout << message << std::endl;
        }
};

class ErrorObserver : public Observer
{
    std::string __path;
    public:
        ErrorObserver(std::string path) :__path{ path } {};
        void onError(const std::string& message) override
        {
            std::ofstream f(__path);
            if (f.is_open()) {
                f << message << std::endl;
                f.close();
            }
            else {
                std::cout << "Не удалось открыть файл для записи." << std::endl;
            }
        }
};

class FatalErrorObserver : public Observer
{
    std::string __path;
public:
    FatalErrorObserver(std::string path) :__path{ path } {};
    void onFatalError(const std::string& message) override
    {
        std::ofstream f(__path);
        if (f.is_open()) {
            f << message << std::endl;
            f.close();
        }
        else {
            std::cout << "Не удалось открыть файл для записи." << std::endl;
        }

        std::cout << message << std::endl;
    }
};

int main()
{
    LogObserver log;

    WarningObserver warningObs;
    ErrorObserver errorObs("error_log.txt");
    FatalErrorObserver fatalErrorObs("fatal_error_log.txt");

    log.add_observer(&warningObs);
    log.add_observer(&errorObs);
    log.add_observer(&fatalErrorObs);

    log.warning("Warring Error!!!");
    log.error("Error!!!");
    log.fatalError("FatalError!!!");

    log.remove_observer(&warningObs);
    log.remove_observer(&errorObs);
    log.remove_observer(&fatalErrorObs);

    return EXIT_SUCCESS;
}