 
#ifndef GAME_SERVER_ENTITIES_DOOR_H
#define GAME_SERVER_ENTITIES_DOOR_H

#include <game/server/entity.h>

class CGameWorld;

class CDoor : public CEntity
{
	vec2 m_To;
	void ResetCollision();
	int m_Length;
	vec2 m_Direction;

public:
	CDoor(CGameWorld *pGameWorld, vec2 Pos, float Rotation, int Length,
		int Number);

	void Reset() override;
	void Snap(int SnappingClient) override;
};

#endif  
