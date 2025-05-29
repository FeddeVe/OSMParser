#pragma once
#include "pch.h" 
 
#define THREAD(host, func) FVEngine::threadPool->addQuee(std::bind(&func, host));

namespace FVEngine {
	class ThreadTask {
		public:
			ThreadTask(std::function<void()> function);
			~ThreadTask();
			std::shared_future<void> getFuture() { return m_future; };
			std::function<void()> m_task;
			void setDone(); 

			void run();
	private:

		std::promise<void> m_promise;
		std::shared_future<void> m_future;
	 	
	}; 

	class ThreadPool
	{
	public:
		ThreadPool(size_t num_threads = std::thread::hardware_concurrency());
		~ThreadPool(); 
		
		std::shared_future<void> addQuee(std::function<void()> task); 
		std::shared_future<void> busyFuture() { return m_threadPoolIsFinished; };

	private:
		std::vector<std::thread> m_threadList;
		std::queue<std::shared_ptr<ThreadTask>> m_taskList;
		std::mutex m_queueMutex;
		std::condition_variable m_cv;
		std::shared_future<void> m_threadPoolIsFinished;
		std::promise<void> m_isBusy;
		bool m_exit; 
	};
};

