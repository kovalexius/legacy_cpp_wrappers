#ifndef CJTHREADPOOL_H
#define CJTHREADPOOL_H

#include <stdio.h>
#include <list>
#include <queue>
#include <memory>
#include <process.h>

#include "Lockable.h"


class CJThreadPool
{
public:
	CJThreadPool( const int &numOfThrds );
	~CJThreadPool();

	void AddShedule( void* (*)( void *t ), void *returnType, void *args );
	void Run(void);
private:
	void workThread(void);

	std::queue< std::shared_ptr< std::pair<void*, void*> > > params;
	std::queue< void* ( *)( void *t ) > shedules;
	std::list< uintptr_t > threads;

	CMutex mutex;
	
	int maxNumOfThreads;
	int numOfThreads;
	int numOfShedules;
};

#endif