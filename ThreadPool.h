#pragma once

#include <thread>
#include <vector>
#include <deque>
#include <mutex>
#include <condition_variable>

class ThreadPool {
private:
	std::vector<std::thread> threads;
	std::deque<std::function<void()>> tasks;

	// synchronization
	std::mutex queue_mutex;
	std::condition_variable condition;
	bool waitStop;
	bool joinStop;
	int readyThreads;

public:
	// constructor just launches some amount of threads 
	ThreadPool(int nthreads) {
		waitStop = false;
		joinStop = false;
		readyThreads = nthreads;
		for (int i = 0; i < nthreads; ++i) {
			threads.emplace_back(std::thread([this] {this->doTask(); }));
		}
	}

	void doTask() {
		while (true) {
			std::function<void()> task;

			{
				std::unique_lock<std::mutex> lock(queue_mutex);
				while (!(waitStop || !tasks.empty() || joinStop)) {
					condition.wait(lock);
				}
				if (joinStop) {
					while (readyThreads != threads.size());
					return;
				}
				if (waitStop && tasks.empty()) return;
				task = move(tasks.front());
				tasks.pop_front();
			}
			task();
			if (joinStop) {
				readyThreads++;
				return;
			}
		}
	}

	//add new task to deque
	void addTask(std::function<void()> task) {
		std::unique_lock<std::mutex> lock(queue_mutex);
		if (waitStop || joinStop) {
			printf("New task is impossible to add\n");
		}
		else {
			tasks.emplace_back(task);
			condition.notify_one();
		}
	}

	// join all threads and ignore remainder tasks
	void joinAll() {
		joinStop = true;
		condition.notify_all();
		int num = threads.size();
		if (waitStop) {
			readyThreads = 0;
		}
		// wait until number of free threads become equal general number of threads
		while (num != readyThreads);
		tasks.clear();
		for (int i = 0; i < num; ++i) {
			if (threads.back().joinable()) {
				threads.back().join();
			}
			threads.pop_back();
			printf("Thread %d was joined\n", i);
		}
		readyThreads = 0;
	}

	//complete all tasks in dequeu and join threads
	void waitAll() {
		waitStop = true;
		condition.notify_all();
		int num = threads.size();
		for (int i = 0; i < num; ++i) {
			if (threads.back().joinable()) {
				threads.back().join();
			}
			threads.pop_back();
			printf("Thread %d was joined\n", i);
		}

	}

	// just join all threads
	~ThreadPool() {
		joinAll();
	}

};