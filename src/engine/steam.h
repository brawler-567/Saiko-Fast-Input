#ifndef ENGINE_STEAM_H
#define ENGINE_STEAM_H

#include "kernel.h"

#include <base/types.h>

class ISteam : public IInterface
{
	MACRO_INTERFACE("steam")
public:
	 
	virtual const char *GetPlayerName() = 0;

	 
	 
	virtual const NETADDR *GetConnectAddress() = 0;
	virtual void ClearConnectAddress() = 0;

	virtual void Update() = 0;

	virtual void ClearGameInfo() = 0;
	virtual void SetGameInfo(const NETADDR &ServerAddr, const char *pMapName, bool AnnounceAddr) = 0;
};

ISteam *CreateSteam();

#endif  
