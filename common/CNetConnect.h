#ifndef CNETCONNECT_H
#define CNETCONNECT_H

#include <string>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <conio.h>


#ifdef _WIN32
#include  <WinSock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib,"Ws2_32.lib")
#else
#include <sys/socket.h>
#endif


#include "CNetUdp.h"
#include "CNetTcp.h"




template<typename T>
class CNetConnect
{
public:
	CNetConnect( T *obj, int port, CNetOptions::SENDOPT opt, CNetOptions::RECEIVEOPT recopt );
	CNetConnect( T *obj, int port, const string& addr, CNetOptions::RECEIVEOPT recopt, CNetOptions::PROTOCOL protcl );
	~CNetConnect();
	bool send( const string& str );
	bool send( const string& str, const string& addres );
	string& get( string& from );
	void setReceiveEvent( void ( T::*ptr_callback) ( const string& str, const string& from ) );
	void setDefaultDestination( const string& addr);
	static void printMessage();
private:
	CNet<T>* net;
};

#endif


template<typename T>
CNetConnect<T>::CNetConnect(  T *obj, int port, CNetOptions::SENDOPT opt = CNetOptions::SENDOPT::BROADCAST, 
	CNetOptions::RECEIVEOPT ropt = CNetOptions::RECEIVEOPT::BLOCK )
{
	net = static_cast< CNet<T>* >( new CNetUdp<T>( obj, 7654, opt, ropt) );
}


template<typename T>
CNetConnect<T>::CNetConnect(  T *obj, int port, const string& ipaddress, 
	CNetOptions::RECEIVEOPT recopt =  CNetOptions::RECEIVEOPT::BLOCK, 
	CNetOptions::PROTOCOL protocol = CNetOptions::PROTOCOL::UDP )
{
	
	switch(protocol)
	{
		case CNetOptions::PROTOCOL::UDP:
			net = dynamic_cast< CNet<T>* >( new CNetUdp<T>( obj, 7654, ipaddress, recopt) );
			break;
		case CNetOptions::PROTOCOL::TCP:
			net = dynamic_cast< CNet<T>* >( new CNetTcp<T>( obj, 7654, ipaddress, recopt) );
			break;
	};
}


template<typename T>
CNetConnect<T>::~CNetConnect()
{
	if( net )
		delete net;
}


template<typename T>
void CNetConnect<T>::setDefaultDestination( const string& ipaddress )
{
	net->setDefaultDestination( ipaddress );
}

template<typename T>
bool CNetConnect<T>::send( const string& srt )
{
	return net->send( srt );
}


template<typename T>
bool CNetConnect<T>::send( const string& str, const string& dest )
{
	return net->send( str, dest );
}


template<typename T>
string& CNetConnect<T>::get( string& from )
{
	return net->get( from );
}


template<typename T>
void CNetConnect<T>::printMessage()
{
	CNet::printMessage();
}


template<typename T>
void CNetConnect<T>::setReceiveEvent( void ( T::*ptr_callback) ( const string& str, const string& from ) )
{
	net->setReceiveEvent( ptr_callback );
}