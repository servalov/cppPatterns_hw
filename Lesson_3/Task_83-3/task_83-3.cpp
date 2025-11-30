// Задание 3. Паттерн «Цепочка ответственности»
#include <iostream>
#include <fstream>

// Типы сообщений
enum class Type
{
    WARNING,
    ERROR,
    FATAL_ERROR,
    UNKNOWN
};

// Класс сообщение о логировании
class LogMessage
{
    private:
        Type _type;
        std::string _message;
    public:
        LogMessage(Type type, const std::string& message) :_type{ type }, _message{ message } {};
        Type type() const
        {
            return _type;
        };
        const std::string& message() const
        {
            return _message;
        };
};

class Handler
{
    public:
        virtual void handler_request(LogMessage* handler) {}
        virtual void set_next_handler(Handler* handler) {};
};

class Warning : public Handler
{
    Handler* next_handler{ nullptr };
    public:
        void set_next_handler(Handler* handler) override
        {
            next_handler = handler;
        }
        void handler_request(LogMessage* handler) override
        {
            if (handler->type() == Type::WARNING)
            {
                std::cout << "Warning: " << handler->message() << std::endl;
            }
            else if (next_handler!=nullptr)
            {
                next_handler->handler_request(handler);
            }
            else
            {
                std::cout << " Uknown type of error!!!" << std::endl;
            }
        }
};

class Error : public Handler
{
    Handler* next_handler{ nullptr };
public:
    void set_next_handler(Handler* handler) override
    {
        next_handler = handler;
    }
    void handler_request(LogMessage* handler) override
    {
        if (handler->type() == Type::ERROR)
        {
            std::ofstream f("error.txt");
            if (f.is_open())
            {
                f << "Error: " << handler->message() << std::endl;
                f.close();
            }
            else
            {
                std::cout << "File open error!!!" << std::endl;
            }
        }
        else if (next_handler != nullptr)
        {
            next_handler->handler_request(handler);
        }
        else
        {
            std::cout << " Uknown type of error!!!" << std::endl;
        }
    }
};

class Fatal_Error : public Handler
{
    Handler* next_handler{ nullptr };
    
    public:
    void set_next_handler(Handler* handler) override
    {
        next_handler = handler;
    }
    void handler_request(LogMessage* handler) override
    {
        if (handler->type() == Type::FATAL_ERROR)
        {
            throw std::runtime_error("Fatal Error: " + handler->message());
        }
        else if (next_handler != nullptr)
        {
            next_handler->handler_request(handler);
        }
        else
        {
            std::cout << " Uknown type of error!!!" << std::endl;
        }
    }
};

class Uknown : public Handler
{
    Handler* next_handler{ nullptr };

public:
    void set_next_handler(Handler* handler) override
    {
        next_handler = handler;
    }
    void handler_request(LogMessage* handler) override
    {
        if (handler->type() == Type::UNKNOWN)
        {
            throw std::runtime_error("Unknown Error: " + handler->message());
        }
        else if (next_handler != nullptr)
        {
            next_handler->handler_request(handler);
        }
        else
        {
            std::cout << " Uknown type of error!!!" << std::endl;
        }
    }
};

int main()
{
    Warning warning_handler;
    Error error_handler;
    Uknown uknown_handler;
    Fatal_Error fatal_handler;

    fatal_handler.set_next_handler(&error_handler);
    error_handler.set_next_handler(&warning_handler);
    warning_handler.set_next_handler(&uknown_handler);

    LogMessage message1{Type::WARNING,"This is warning error." };
    fatal_handler.handler_request(&message1);

    LogMessage message2{ Type::UNKNOWN,"This is unknown error." };
    fatal_handler.handler_request(&message2);
    
    return EXIT_SUCCESS;
}