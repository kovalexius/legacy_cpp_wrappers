#ifndef CNETUDP_H
#define CNETUDP_H



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
class CNetUdp: public CNet<T>
{
	friend void WINAPI receiveUdp( LPVOID lpParameter );
public:
	CNetUdp( T *obj, int port, CNetOptions::SENDOPT opt, CNetOptions::RECEIVEOPT ropt );
	CNetUdp( T *obj, int port, const string& straddr, CNetOptions::RECEIVEOPT ropt );
	~CNetUdp();

	virtual bool send( const string& str );
	virtual bool send( const string& str, const string& dest );
	virtual string& get( string& from );
	virtual void setReceiveEvent( void ( T::*ptr) ( const string& str, const string& from ) );
private:
	CNetOptions::SENDOPT sendopt;
	void initBroadcast();
	void initMulticast();

	void receive( void );
};

#endif

template<typename T>
CNetUdp<T>::CNetUdp( T *obj, int port, CNetOptions::SENDOPT opt, CNetOptions::RECEIVEOPT ropt ): CNet<T>( obj, port, ropt )
{
	sendopt = opt;
	receiveopt = ropt;

	switch( sendopt )
	{
		case CNetOptions::BROADCAST:
			initBroadcast( );
			break;
		case CNetOptions::MULTICAST:
			initMulticast( );
			break;
	};
}

template<typename T>
CNetUdp<T>::CNetUdp( T *obj, int port, const string& ipaddress, CNetOptions::RECEIVEOPT ropt ): CNet<T>( obj, port, ropt )
{
	s_in = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );
	s_out = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );

	const char optval = 1;
	setsockopt( s_in, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(BOOL) );

	address_len = sizeof(struct sockaddr_in);

	setDefaultDestination( ipaddress );

	memset( &in, 0, sizeof(in) );
	in.sin_family = PF_INET;
	in.sin_addr.s_addr = htonl(INADDR_ANY);
	in.sin_port = htons( port );

	if( bind( s_in, (sockaddr *)&in, sizeof(in) ) != SOCKET_ERROR )
	{
	}
	else
		printMessage();

	u_long iMode = 0;
	int iResult;
	if( receiveopt == CNetOptions::UNBLOCK )
		iMode = 1;
	else
		iMode = 0;

	iResult = ioctlsocket(s_in, FIONBIO, &iMode);
	if (iResult != NO_ERROR)
		printMessage();
}

template<typename T>
void CNetUdp<T>::initBroadcast( )
{
	s_in = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );
	s_out = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );

	const char optval = 1;
	setsockopt( s_in, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(BOOL) );
	setsockopt( s_out, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(char) );

	address_len = sizeof(struct sockaddr_in);
	
	memset( &out, 0, sizeof(out) );
	out.sin_family = PF_INET;
  out.sin_port = htons( port );
	out.sin_addr.s_addr = htonl(INADDR_BROADCAST);

	memset( &in, 0, sizeof(in) );
	in.sin_family = PF_INET;
  in.sin_port = htons( port );
	in.sin_addr.s_addr = htonl(INADDR_ANY);

	if( bind( s_in, ( struct sockaddr* )&in, sizeof( struct sockaddr ) ) != SOCKET_ERROR )
	{
	}
	else
		printMessage();

	u_long iMode = 0;
	int iResult;
	if( receiveopt == CNetOptions::UNBLOCK )
		iMode = 1;
	else
		iMode = 0;

	iResult = ioctlsocket(s_in, FIONBIO, &iMode);
	if (iResult != NO_ERROR)
		printMessage();

}


template<typename T>
void CNetUdp<T>::initMulticast()
{
}


template<typename T>
bool CNetUdp<T>::send( const string& srt )
{
	char sbuf[1024];
	sprintf( sbuf, "%s", srt.c_str() );
	int ret = sendto( s_out, sbuf, strlen(sbuf) + 1, 0, (sockaddr*)&out, sizeof(out) );

	return true;
}


template<typename T>
bool CNetUdp<T>::send( const string& srt, const string& dest )
{
	sockaddr_in to_dest;

	memset( &to_dest, 0, sizeof(to_dest) );
	to_dest.sin_family = PF_INET;
	to_dest.sin_addr.s_addr = inet_addr( dest.c_str() );
	to_dest.sin_port = htons( port );

	char sbuf[1024];

	sprintf( sbuf, "%s", srt.c_str() );
	int ret = sendto( s_out, sbuf, strlen(sbuf) + 1, 0, (sockaddr*)&to_dest, sizeof(to_dest) );

	return true;
}


template<typename T>
string& CNetUdp<T>::get( string& from )
{
	char Hz[1024];
	int recsize;
	char *ip;
	string in_str;

	FD_SET ReadSet;
	FD_ZERO(&ReadSet);
	FD_SET(s_in, &ReadSet);
	TIMEVAL tval = { 0, 0 };
	int res = select( 0, &ReadSet, NULL, NULL, &tval );

	in_str = "";
	from = "";

	if( res != SOCKET_ERROR )
	{
		recsize = recvfrom( s_in, Hz, sizeof(Hz), 0, ( SOCKADDR * )&addr, &address_len );
		if ( recsize > 0 )
		{
			in_str = Hz;
			ip = inet_ntoa( addr.sin_addr );
			from = (const char*)ip;
		}			
	}
	
	return in_str;
}


template<typename T>
void CNetUdp<T>::setReceiveEvent( void ( T::*ptr) ( const string& str, const string& from ) )
{
	ptr_callback = ptr;
	//isCallBack = true;
	DWORD dwThreadId;

	if(hThread)
	{
		CloseHandle( hThread );
		hThread = 0;
	}
	hThread = CreateThread(NULL, 0, 
		(LPTHREAD_START_ROUTINE)thread_to_member_thunk< CNetUdp<T>, &CNetUdp<T>::receive >, 
		this, 0, &dwThreadId);
}

template<typename T>
void CNetUdp<T>::receive()
{
	char Hz[1024];
	char *ip;
	string from, in_str;

	while(1)
	{
		FD_SET ReadSet;
		FD_ZERO(&ReadSet);
		FD_SET(s_in, &ReadSet);
		TIMEVAL tval = { 0, 0 };

		in_str = "";
		from = "";
		int res = select( 0, &ReadSet, NULL, NULL, &tval );
		if( res != SOCKET_ERROR )
		{
			int recsize = recvfrom( s_in, Hz, sizeof(Hz), 0, ( SOCKADDR * )&addr, &address_len );
			if ( recsize > 0 )
			{
				in_str = Hz;
				ip = inet_ntoa( addr.sin_addr );
				from = (const char*)ip;

				(client_obj->*ptr_callback)(in_str, from);
			}
		}
		else
			printMessage();
	}
}


template<typename T>
CNetUdp<T>::~CNetUdp()
{

}