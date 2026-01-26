#ifndef THREAD_POOL
#define THREAD_POOL

#include <iostream>
#include <vector>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>
#include <future>
#include <chrono>

using type_task = std::packaged_task<void(std::string&,int&)>;

// Задача с url адресом и глубиной 
struct url_task
{
	type_task task;
	std::string url;
	int url_depth;	
};

// Шаблонный класс для потокобезопасной очереди задач
template <class T>
class Safe_queue
{
private:
	std::queue<T> task_queue;               // очередь задач
	std::mutex mtx;                         // мьютекс для доступа
	std::condition_variable notice;
	bool stop_request{ false };             // признак остановки

public:
	Safe_queue();
	void push(T func);            // Метод push для добавлении задачи в очередь
	bool pop(T& task);            // Метод pop для извлечения задачи из очереди
	void stop_threads();          // Остановка потоков
};

// Класс для реализации пула потоков
class Thread_pool
{
private:
	std::vector<std::thread> threads_vector;
	Safe_queue<url_task> tasks_queue;
	size_t num_threads{};
	std::mutex mtx_pool;

public:
	Thread_pool(size_t numThreads);
	~Thread_pool();
	void work();
	std::future<void> submit(type_task task, std::string& url, int& url_depth);
};

#endif // THREAD_POOL