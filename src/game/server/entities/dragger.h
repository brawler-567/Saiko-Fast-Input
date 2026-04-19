 
#ifndef GAME_SERVER_ENTITIES_DRAGGER_H
#define GAME_SERVER_ENTITIES_DRAGGER_H

#include <game/server/entity.h>
class CDraggerBeam;

 
class CDragger : public CEntity
{
	 
	vec2 m_Core;
	float m_Strength;
	bool m_IgnoreWalls;
	int m_EvalTick;

	int m_aTargetIdInTeam[MAX_CLIENTS];
	CDraggerBeam *m_apDraggerBeam[MAX_CLIENTS];

	void LookForPlayersToDrag();

public:
	CDragger(CGameWorld *pGameWorld, vec2 Pos, float Strength, bool IgnoreWalls, int Layer = 0, int Number = 0);

	void RemoveDraggerBeam(int ClientId);
	bool WillDraggerBeamUseDraggerId(int TargetClientId, int SnappingClientId);

	void Reset() override;
	void Tick() override;
	void Snap(int SnappingClient) override;
	void SwapClients(int Client1, int Client2) override;
};

#endif  
