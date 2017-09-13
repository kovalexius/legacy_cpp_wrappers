#ifndef CNETTCP_H
#define CNETTCP_H


#ifdef _WIN32
#include  <WinSock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib,"Ws2_32.lib")
#else
#include <sys/socket.h>
#endif


#include "CNet.h"


template<typename T>
class CNetTcp: public CNet<T>
{
public:
	CNetTcp( T *obj, int port, const string& ipaddress, CNetOptions::RECEIVEOPT recopt );
	~CNetTcp();
	string& get( string& from );
	bool send( const string& str );
	bool send( const string& str, const string& dest );
	void setReceiveEvent( void ( T::*ptr) ( const string& str, const string& from ) );
protected:
	void initOutSocket();
	string& getBlock( string& from );
	string& getUnBlock( string& from );
	SOCKET accpt;
	bool connected;
	bool accepted;
	void receive( void );
};

#endif


template<typename T>
CNetTcp<T>::CNetTcp( T *obj, int port, const string& ipaddress, CNetOptions::RECEIVEOPT ropt ): CNet<T>( obj, port, ropt )
{
	s_in = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
	
	const char optval = 1;
	setsockopt( s_in, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(BOOL) );
	setsockopt( s_in, SOL_SOCKET, SO_OOBINLINE, &optval, sizeof(BOOL) );

	address_len = sizeof(struct sockaddr_in);

	setDefaultDestination( ipaddress );
  
	memset( &in, 0, sizeof(in) );
	in.sin_family = PF_INET;
	in.sin_addr.s_addr = htonl(INADDR_ANY);
	in.sin_port = htons( this->port );

	if( bind( s_in, (sockaddr *)&in, sizeof(in) ) != SOCKET_ERROR )
	{
	}
	else
		printMessage();

	connected = false;
	accepted = false;

	u_long iMode = 0;
	int iResult;
	if( receiveopt == CNetOptions::UNBLOCK )
	{
		iMode = 1;
		ptr_get = (string&(CNet::*)( string& )) &CNetTcp::getUnBlock;
	}
	else
	{
		iMode = 0;
		ptr_get = (string&(CNet::*)( string& )) &CNetTcp::getBlock;
	}

	iResult = ioctlsocket(s_in, FIONBIO, &iMode);
	if (iResult != NO_ERROR)
		printMessage();
}


template<typename T>
void CNetTcp<T>::initOutSocket()
{
	s_out = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
	const char optval = 1;
	setsockopt( s_out, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(BOOL) );
	setsockopt( s_out, SOL_SOCKET, SO_OOBINLINE, &optval, sizeof(BOOL) );
}


template<typename T>
bool CNetTcp<T>::send( const string& str )
{
	if( !connected )
	{
		initOutSocket();
		connected = true;
	}
	connect( s_out, ( const sockaddr* )&out, sizeof( out ));
	
	char sbuf[1024];
	sprintf( sbuf, "%s", str.c_str() );
	int ret = sendto( s_out, sbuf, strlen(sbuf) + 1, 0, (sockaddr*)&out, sizeof(out) );

	if( ret == SOCKET_ERROR)
	{
		shutdown( s_out, SD_BOTH );
		closesocket( s_out );
		connected = false;
	}
	return connected;
}



template<typename T>
bool CNetTcp<T>::send( const string& str, const string& dest )
{
	sockaddr_in to_dest;

	memset( &to_dest, 0, sizeof( to_dest ) );
	to_dest.sin_family = PF_INET;
  to_dest.sin_port = htons( this->port );
	to_dest.sin_addr.s_addr = inet_addr( dest.c_str() );

	if( connect( s_out, ( const sockaddr* )&to_dest, sizeof( to_dest )) == SOCKET_ERROR )
	{
		shutdown( s_out, SD_BOTH );
		closesocket( s_out );
		initOutSocket();
		connect( s_out, ( const sockaddr* )&to_dest, sizeof( to_dest ));
	}
	connected = true;
	
	char sbuf[1024];
	sprintf( sbuf, "%s", str.c_str() );
	int ret = sendto( s_out, sbuf, strlen(sbuf) + 1, MSG_DONTROUTE, (sockaddr*)&to_dest, sizeof(to_dest) );

	if( ret == SOCKET_ERROR)
	{
		shutdown( s_out, SD_BOTH );
		closesocket( s_out );
		connected = false;
	}

	return connected;
}



template<typename T>
string& CNetTcp<T>::get( string& from )
{
	return (this->*ptr_get)( from );
}


template<typename T>
string& CNetTcp<T>::getBlock( string& from)
{
	char Hz[1024];
	int recsize;
	char *ip;
	string in_str;

	if( listen( s_in, SOMAXCONN ) == SOCKET_ERROR	)
		printMessage();

	accpt = accept( s_in, 0, 0 );

	if( accpt == SOCKET_ERROR )
		printMessage();
	
	in_str = "";
	from = "";

	recsize = recvfrom( accpt, Hz, sizeof(Hz), 0, ( SOCKADDR * )&addr, &address_len );
	if( recsize > 0 )
	{
		in_str = Hz;
		ip = inet_ntoa( addr.sin_addr );
		from = (const char*)ip;
	}

	shutdown( accpt, SD_BOTH );
	closesocket( accpt );
	
	return in_str;
}


template<typename T>
string& CNetTcp<T>::getUnBlock( string& from )
{
	char Hz[1024];
	int recsize;
	char *ip;
	string in_str;

	if( listen( s_in, SOMAXCONN ) == SOCKET_ERROR	)
		printMessage();

	accpt = accept( s_in, (sockaddr *)&addr, &address_len);

	if( accpt < 0 )
		printMessage();

	FD_SET ReadSet;
	FD_ZERO(&ReadSet);
	FD_SET(s_in, &ReadSet);
	TIMEVAL tval = { 0, 0 };
	int res = select( 0, &ReadSet, NULL, NULL, &tval );
	
	if( res != SOCKET_ERROR )
	{
		recsize = recvfrom( accpt, Hz, sizeof(Hz), 0, ( SOCKADDR * )&addr, &address_len );
		if( recsize > 0 )
		{
			in_str = Hz;
			ip = inet_ntoa( addr.sin_addr );
			from = (const char*)ip;
		}
	}
	else
		printMessage();

	shutdown( accpt, SD_BOTH );
	closesocket( accpt );
	
	return in_str;
}


template<typename T>
void CNetTcp<T>::receive( void )
{
	char Hz[1024];
	char* ip;
	string from;
	string in_str;

	while(1)
	{
		in_str = "";
		from = "";

		FD_SET ReadSet;
		FD_ZERO(&ReadSet);
		FD_SET(s_in, &ReadSet);
		TIMEVAL tval = { 0, 0 };

		int res = select( 0, &ReadSet, NULL, NULL, &tval );
		
		if( res != SOCKET_ERROR )
		{
			accpt = accept( s_in, (sockaddr *)&addr, &address_len);
			if( accpt != INVALID_SOCKET )
			{
				int recsize = recvfrom( accpt, Hz, sizeof(Hz), 0, ( SOCKADDR * )&addr, &address_len );
				if( recsize > 0 )
				{
					in_str = Hz;
					ip = inet_ntoa( addr.sin_addr );
					from = (const char*)ip;
	
					(client_obj->*ptr_callback)(in_str, from);
				}
			}
		}
		else
		{
			printMessage();
			shutdown( accpt, SD_BOTH );
			closesocket( accpt );
		}
	}

}


template<typename T>
void CNetTcp<T>::setReceiveEvent( void ( T::*ptr) (const string& str, const string& from ) )
{
	ptr_callback = ptr;
	//isCallBack = true;
	DWORD dwThreadId;

	if(hThread)
	{
		CloseHandle( hThread );
		hThread = 0;
	}
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thread_to_member_thunk< CNetTcp<T>, 
		&CNetTcp<T>::receive > , this, 0, &dwThreadId);

	if( listen( s_in, SOMAXCONN ) == SOCKET_ERROR	)
			printMessage();
}


template<typename T>
CNetTcp<T>::~CNetTcp()
{

}

