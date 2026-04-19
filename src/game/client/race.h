#ifndef GAME_CLIENT_RACE_H
#define GAME_CLIENT_RACE_H

#include <base/vmath.h>

class CGameClient;

class CRaceHelper
{
	const CGameClient *m_pGameClient;

	int m_aFlagIndex[2] = {-1, -1};

public:
	void Init(const CGameClient *pGameClient);

	 
	static int TimeFromSecondsStr(const char *pStr);  
	static int TimeFromStr(const char *pStr);  
	static int TimeFromFinishMessage(const char *pStr, char *pNameBuf, int NameBufSize);  

	bool IsStart(vec2 Prev, vec2 Pos) const;
};

#endif  
