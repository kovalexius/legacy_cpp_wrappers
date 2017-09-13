#ifndef JSHOT_H
#define JSHOT_H

extern "C"
{
	__declspec(dllexport)	void** getJShot( int quality, int maxHeight,
															unsigned int *&lensBuf, unsigned int *&y, unsigned int *&h, bool *&isNew, int &count, int &width, int &height );

	__declspec(dllexport) int printHelloWorld( int time );
	__declspec(dllexport) bool destroyJShot( void );
	__declspec(dllexport)	void destroyBuffers( void );
}

#endif