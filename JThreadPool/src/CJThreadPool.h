#ifndef CJTHREADPOOL_H
#define CJTHREADPOOL_H

#include <list>
#include <queue>
#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

#include <stdio.h>


class CJThreadPool
{
public:
	CJThreadPool(const int& _numOfThrds);
	CJThreadPool(const CJThreadPool &) = delete;
	~CJThreadPool();

	void AddShedule(void* (*)( void *t), void *_returnType, void *_args);
	void Run();
private:
	static void workThread(CJThreadPool& _obj);
	void schedule();

	std::queue<std::shared_ptr<std::pair<void*, void*>>> m_params;
	std::queue<void* ( *)( void *t )> m_shedules;
	std::vector<std::thread> m_threads;

	std::mutex m_mutex;
	std::atomic<bool> m_stillRunning;
	
	int m_maxNumOfThreads;
	int m_numOfThreads;
	int m_numOfShedules;
};

#endif