#include "thread_pool.h"

template <class T> Safe_queue<T>::Safe_queue()
{

}

// Метод push для добавлении задачи в очередь
template <class T> void Safe_queue<T>::push(T func)
{
	std::unique_lock<std::mutex> lk(mtx);
	task_queue.emplace(std::move(func));
	//std::cout << "\n В очередь добавлена новая задача!!!" << std::endl;
	lk.unlock();
	notice.notify_one();
}

// Метод pop для извлечения задачи из очереди
template <class T> bool Safe_queue<T>::pop(T& task)
{
	std::unique_lock<std::mutex> lk(mtx);
	notice.wait(lk, [this]() {return !task_queue.empty() || stop_request; });

	if (task_queue.empty() && stop_request)
	{
		std::cout << " Очередь пуста. ";
		return false;
	}

	task = std::move(task_queue.front());
	task_queue.pop();
	lk.unlock();
	return true;
}

template <class T> void Safe_queue<T>::stop_threads()
{
	std::unique_lock<std::mutex> lk(mtx);
	stop_request = true;
	notice.notify_all();
}

void Thread_pool::work()
{
	while (true)
	{
		url_task new_task;
		if (!tasks_queue.pop(new_task))
		{
			std::cout << "   Работа потока " << std::this_thread::get_id() << " завершена " << std::endl;
			break;
		}
		new_task.task(new_task.url,new_task.url_depth);
	}
}

Thread_pool::~Thread_pool()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	//std::cout << "\n Удаление объекта thread_pool:" << std::endl;
	tasks_queue.stop_threads();
	for (size_t i{}; i < threads_vector.size(); ++i)
	{
		threads_vector[i].join();
	}
}

Thread_pool::Thread_pool(size_t numThreads) : num_threads(numThreads)
{
	std::cout << "\n Создание объекта thread_pool (пул потоков)!!!" << std::endl;
	std::cout << " Число рабочих потоков: " << num_threads << std::endl;
	threads_vector.reserve(num_threads);

	for (size_t i{}; i < num_threads; ++i)
	{
		threads_vector.emplace_back(std::thread(&Thread_pool::work, this));
	}
}

std::future<void> Thread_pool::submit(type_task task, std::string& url, int& url_depth)
{
	std::future<void> result = task.get_future();
	tasks_queue.push({ std::move(task), url ,url_depth});
	return result;
}
