#include <chrono>
#include "CJThreadPool.h"

CJThreadPool::CJThreadPool(const int& _numOfThrds) : 
									m_maxNumOfThreads(_numOfThrds),
									m_numOfThreads(0),
									m_numOfShedules(0),
									m_stillRunning(true)
{
}

CJThreadPool::~CJThreadPool()
{
	m_stillRunning.store(false);
}

void CJThreadPool::AddShedule( void* ( *f_ptr)(void *), void *returnType, void *args )
{
	m_shedules.push( f_ptr );
	m_params.push( std::shared_ptr<std::pair<void*,void*>>( new std::pair<void*,void*> ( returnType, args) ) );
	m_numOfShedules++;
	
	if( m_numOfThreads < m_maxNumOfThreads )
		m_numOfThreads++;
}

//template<typename T, void(T::*mem_fun)()>
//unsigned int WINAPI thread_to_member_thunk(void* p)
//{
//   (static_cast<T*>(p)->*mem_fun)();
//	 return 3;
//}

void CJThreadPool::Run()
{
	for(int i = 0; i<m_numOfThreads; i++)
	{
		//uintptr_t hThread;
		//hThread = _beginthreadex(NULL, 0, workThread, *this, 0, NULL);
		//m_threads.push_back( hThread );
		//m_threads.push_back(std::make_shared<std::thread>(workThread, *this));
		std::thread(workThread, std::ref(*this)).detach();
	}

	
	//for( auto it = m_threads.begin(); it != m_threads.end(); it ++ )	// Ждем все потоки, после выходим отсюда
	//{
	//	WaitForSingleObject( (HANDLE)*it, INFINITE );
	//	CloseHandle( (HANDLE)*it );
	//}
	
}

void CJThreadPool::workThread(CJThreadPool& _obj)
{
	
		_obj.schedule();
}

void CJThreadPool::schedule()
{
	while (m_stillRunning)
	{
		void * (*pf) (void*);
		pf = 0;
		std::shared_ptr<std::pair<void*, void*>> par;

		{
			std::unique_lock<std::mutex> lk(m_mutex);
			if (m_shedules.empty())
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				continue;
			}
			par.swap(m_params.front());
			pf = m_shedules.front();
			m_shedules.pop();
			m_params.pop();
			m_numOfShedules--;
		}

		if (pf)
			par->first = pf(par->second);  // Вызов вычислений
	}
}