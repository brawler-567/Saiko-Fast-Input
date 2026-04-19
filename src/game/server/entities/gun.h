 

#ifndef GAME_SERVER_ENTITIES_GUN_H
#define GAME_SERVER_ENTITIES_GUN_H

#include <game/server/entity.h>

 
class CGun : public CEntity
{
	vec2 m_Core;
	bool m_Freeze;
	bool m_Explosive;
	int m_EvalTick;
	int m_aLastFireTeam[MAX_CLIENTS];
	int m_aLastFireSolo[MAX_CLIENTS];

	void Fire();

public:
	CGun(CGameWorld *pGameWorld, vec2 Pos, bool Freeze, bool Explosive, int Layer = 0, int Number = 0);

	void Reset() override;
	void Tick() override;
	void Snap(int SnappingClient) override;
};

#endif  
