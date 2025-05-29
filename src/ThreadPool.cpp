#include "pch.h" 
#include "threadPool.h" 


using namespace FVEngine;


ThreadTask::ThreadTask(std::function<void()> function)
	: m_task{ function }
{
	m_future = m_promise.get_future(); 
}

ThreadTask::~ThreadTask() {
	//LOG_D("THREADTASK DELETED");
}

void ThreadTask::setDone() {
	//LOG_D("TASK SET DONE");
	m_promise.set_value();
}


ThreadPool::ThreadPool(size_t num_threads)
	: m_exit{false}
{

	m_threadPoolIsFinished = m_isBusy.get_future();

	num_threads = num_threads-1;
	if (num_threads > std::thread::hardware_concurrency()) {
		std::cout<< ("Thread pool size is bigger than hardware threads!") << std::endl;
	}

  

	for (size_t i = 0; i < num_threads; i++) {


		m_threadList.emplace_back([this] {
			std::shared_ptr<ThreadTask> task;
			while (1) {
				{

					std::unique_lock<std::mutex> lock(m_queueMutex);
					m_cv.wait(lock, [this] {
						return !m_taskList.empty() || m_exit;
					});

					if (m_exit && m_taskList.empty()) { 
						return;
					}

					task = m_taskList.front();
					m_taskList.pop(); 
					
				}
				task->m_task();
				task->setDone();
				task.reset();
			}
		});
	}
}



ThreadPool::~ThreadPool() {
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		m_exit = true;
	}
	m_cv.notify_all();

	for (auto& thread : m_threadList) {
		// if (thread.joinable()) {
		 	thread.join();
		// }
	}
	m_isBusy.set_value();
}

std::shared_future<void> ThreadPool::addQuee(std::function<void()> task) {
	std::shared_ptr<ThreadTask> treadTask = std::make_shared<ThreadTask>(task);  
	{
		std::unique_lock<std::mutex> lock(m_queueMutex); 
		m_taskList.emplace(treadTask);
		//std::string msg = "ThreadPool TaskList Size : " + std::to_string(m_taskList.size());
		//LOG_D(msg.c_str());
	}  
	m_cv.notify_one();
	return treadTask.get()->getFuture();
}