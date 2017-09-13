#include "CSocket.h"

CNetConnect::CNetConnect( int port, CNetConnect::OPT opt )
{
	WORD w = MAKEWORD(1,1);
  WSADATA wsadata;
  WSAStartup(w, &wsadata);

	option = opt;

	switch( opt )
	{
		case CNetConnect::BROADCAST:
			initBroadcast( port );
			break;
		case CNetConnect::MULTICAST:
			initBroadcast( port );
			break;
	};
	
	
}


CNetConnect::CNetConnect( int port, const string& addr)
{
}

CNetConnect::~CNetConnect()
{
	closesocket(s);
}

void CNetConnect::initBroadcast(int port)
{
	s = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );

	const char optval = 1;
	setsockopt( s, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(char) );

	memset(&addr,0, sizeof(addr));
	addr.sin_family = AF_INET;
  addr.sin_port = htons( port );
	addr.sin_addr.s_addr = INADDR_BROADCAST;
	len = sizeof(addr);
}

void CNetConnect::initMulticast(int port)
{
	s = socket( AF_INET, SOCK_DGRAM, 0 );

	const char optval = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	memset(&addr, 0, sizeof(addr));
	//bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(s, (sockaddr *)&addr, sizeof(addr));
}


void CNetConnect::send(const string& srt)
{
	char sbuf[1024];
  sprintf( sbuf, "%s\r\n", "" );
	int ret = sendto( s, sbuf, strlen(sbuf), 0, (sockaddr*)&addr, len );

	if(ret < 0)
  {
      std::cout << "Error broadcasting to the clients" << endl;
  }
  else if(ret < strlen(sbuf))
  {
      std::cout << "Not all data broadcasted to the clients" << endl;
  }
  else
  {
      std::cout << "Broadcasting is done" << endl;
  }

	int cerr = WSAGetLastError();
	switch(cerr)
	{
		case WSANOTINITIALISED:
			std::cout << "A successful WSAStartup call must occur before using this function." << endl;
			break;
		case WSAENETDOWN:
			std::cout << "The network subsystem has failed." << endl;
			break;
		case WSAEACCES:
			std::cout << "The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt with the SO_BROADCAST parameter to allow the use of the broadcast address." << endl;
			break;
		case WSAEINVAL:
			std::cout << "An unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled." << endl;
			break;
		case WSAEINTR:
			std::cout << "A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall." << endl;
			break;
		case WSAEINPROGRESS:
			std::cout << "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function." << endl;
			break;
		case WSAEFAULT:
			std::cout << "The buf or to parameters are not part of the user address space, or the tolen parameter is too small." << endl;
			break;
		case WSAENETRESET:
			std::cout << "The connection has been broken due to keep-alive activity detecting a failure while the operation was in progress." << endl;
			break;
	};
}
