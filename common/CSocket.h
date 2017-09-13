#ifndef SOCKET_H
#define SOCKET_H

#include <string>
#include <iostream>

#ifdef _WIN32
#include  <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib,"Ws2_32.lib")
#else
#include <sys/socket.h>
#endif

using namespace std;

class CNetConnect
{
public:
	enum OPT{ MULTICAST, BROADCAST };
	enum RECEIVEOPT { BLOCK, UNBLOCK, CALLBACKING };
	CNetConnect( int port, CNetConnect::OPT opt );
	CNetConnect( int port, const string& addr );
	~CNetConnect();
	void send( const string& str );
private:
	void initBroadcast(int port);
	void initMulticast(int port);
	OPT option;
	RECEIVEOPT recopt;
	SOCKET s;
	int len;
	SOCKADDR_IN addr;
};

#endif