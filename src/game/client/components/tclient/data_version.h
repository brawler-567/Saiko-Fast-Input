#ifndef GAME_CLIENT_COMPONENTS_TCLIENT_DATA_VERSION_H
#define GAME_CLIENT_COMPONENTS_TCLIENT_DATA_VERSION_H
#else
#error data_version.h included multiple times
#endif

 
 
 

 
 

#include <base/system.h>

#include <engine/shared/linereader.h>

#include <game/localization.h>
#include <game/version.h>

#define DATA_VERSION_PATH "data_version.txt"

 
 
 
 

 
 
 
 

inline void CheckDataVersion(char *pError, int Length, IOHANDLE File)
{
	if(!File)
	{
		 
		return;
	}

	 
	 

	 
	 
	 
	 
	 
	 

	io_close(File);
}
