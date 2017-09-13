#ifndef CNETOPTIONS_H
#define CNETOPTIONS_H

struct CNetOptions
{
	enum SENDOPT{ MULTICAST, BROADCAST };
	enum RECEIVEOPT { BLOCK, UNBLOCK };
	enum PROTOCOL { UDP, TCP };
};

#endif