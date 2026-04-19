 
#ifndef GAME_TEAMSCORE_H
#define GAME_TEAMSCORE_H

#include <engine/shared/protocol.h>

enum
{
	TEAM_FLOCK = 0,
	TEAM_SUPER = 64,
	NUM_DDRACE_TEAMS = TEAM_SUPER + 1,
	VANILLA_TEAM_SUPER = VANILLA_MAX_CLIENTS
};

 
enum
{
	SV_TEAM_FORBIDDEN = 0,  
	SV_TEAM_ALLOWED = 1,  
	SV_TEAM_MANDATORY = 2,  
	SV_TEAM_FORCED_SOLO = 3  
};

class CTeamsCore
{
	int m_aTeam[MAX_CLIENTS];
	bool m_aIsSolo[MAX_CLIENTS];

public:
	bool m_IsDDRace16;

	CTeamsCore();

	bool SameTeam(int ClientId1, int ClientId2) const;

	bool CanKeepHook(int ClientId1, int ClientId2) const;
	bool CanCollide(int ClientId1, int ClientId2) const;

	int Team(int ClientId) const;
	void Team(int ClientId, int Team);

	void Reset();
	void SetSolo(int ClientId, bool Value);
	bool GetSolo(int ClientId) const;
};

#endif
