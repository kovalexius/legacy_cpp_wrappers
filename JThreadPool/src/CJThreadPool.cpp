#include "CJThreadPool.h"

CJThreadPool::CJThreadPool( const int &numOfThrds )
{
	maxNumOfThreads = numOfThrds;
	numOfThreads = 0;
	numOfShedules = 0;
}

CJThreadPool::~CJThreadPool()
{

}

void CJThreadPool::AddShedule( void* ( *f_ptr)( void *t ), void *returnType, void *args )
{
	shedules.push( f_ptr );
	params.push( std::shared_ptr<std::pair<void*,void*>>( new std::pair<void*,void*> ( returnType, args) ) );
	numOfShedules++;
	
	if( numOfThreads < maxNumOfThreads )
		numOfThreads++;
}

template<class T, void(T::*mem_fun)()>
unsigned int WINAPI thread_to_member_thunk(void* p)
{
   (static_cast<T*>(p)->*mem_fun)();
	 return 3;
}

void CJThreadPool::Run()
{
	for( int i = 0; i<numOfThreads; i++ )	// Запуск потоков
	{
		uintptr_t hThread;
		hThread = _beginthreadex(NULL, 0, thread_to_member_thunk<CJThreadPool, &CJThreadPool::workThread>, this, 0, NULL );
		//SetThreadPriority( (HANDLE)hThread, THREAD_PRIORITY_TIME_CRITICAL );
		threads.push_back( hThread );
	}

	
	for( auto it = threads.begin(); it != threads.end(); it ++ )	// Ждем все потоки, после выходим отсюда
	{
		WaitForSingleObject( (HANDLE)*it, INFINITE );
		CloseHandle( (HANDLE)*it );
	}
	
}

void CJThreadPool::workThread(void)
{
	while( 1 )
	{
		void * (*pf) (void*);
		pf = 0;

		mutex.Lock();
		if( shedules.empty() )
		{
			mutex.Unlock();
			break;
		}
		std::shared_ptr< std::pair<void*, void*> > par = params.front();
		pf = shedules.front();
		shedules.pop();
		params.pop();
		numOfShedules--;
		mutex.Unlock();

		if(pf)
			par->first = pf( par->second );  // Вызов вычислений
	}
}