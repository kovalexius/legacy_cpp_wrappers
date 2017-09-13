#ifndef CNET_H
#define CNET_H

#include <stdio.h>
#include <string>
#include <iostream>
#include <map>

#ifdef _WIN32
#include  <WinSock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib,"Ws2_32.lib")
#else
#include <sys/socket.h>
#endif

#include "Lockable.h"
#include "CNetOptions.h"


using namespace std;

template<typename T>
class CNet
{
public:
	CNet( T *obj, int port, CNetOptions::RECEIVEOPT ropt );
	virtual ~CNet();
	
	virtual bool send	( const string& str ) = 0;
	virtual bool send( const string& str, const string& dest ) = 0;
	virtual string& get( string& from ) = 0;
	virtual void setReceiveEvent( void ( T::* ptr_callback) ( const string& str, const string& from ) ) = 0;
	void setDefaultDestination( const string& addr);

	static void printMessage( void );

protected:
	void ( T::*ptr_callback ) ( const string& str, const string& from );
	T *client_obj;

	string& ( CNet::*ptr_get )( string& from );

	int port;
	CNetOptions::RECEIVEOPT receiveopt;
	SOCKET s_in;
	SOCKET s_out;
	sockaddr_in in, out, addr;
	socklen_t address_len;
	bool isCallBack;

	HANDLE hThread;
};

template<typename T>
void CNet<T>::setDefaultDestination( const string& ipaddress )
{
	memset( &out, 0, sizeof(out) );
	out.sin_family = PF_INET;
	out.sin_addr.s_addr = inet_addr( ipaddress.c_str() );
	out.sin_port = htons( this->port );
}

template<typename T>
CNet<T>::CNet( T *obj, int port, CNetOptions::RECEIVEOPT ropt )
{
	WORD w = MAKEWORD(2,2);
  WSADATA wsadata;
  WSAStartup(w, &wsadata);

	receiveopt = ropt;
	this->port = port;
	client_obj = obj;

	u_long iMode = 0;
	//isCallBack = false;
	hThread = 0;
	s_in = 0;
	s_out = 0;
}


template<typename T>
CNet<T>::~CNet()
{
	if(s_in)
		closesocket(s_in);
	if(s_out)
		closesocket(s_out);
	if(hThread)
		CloseHandle(hThread);
	WSACleanup();
}


template<typename T>
void CNet<T>::printMessage()
{
	TCHAR szBuffer[1024];
	::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, ::WSAGetLastError(), 0, (LPSTR)szBuffer, sizeof(szBuffer), NULL);
	cout << szBuffer << endl;
}


template<class T, void(T::*mem_fun)()>
void WINAPI thread_to_member_thunk(void* p)
{
   (static_cast<T*>(p)->*mem_fun)();
}

#endif