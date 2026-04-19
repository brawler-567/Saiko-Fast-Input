 
#ifndef GAME_SERVER_ENTITIES_DRAGGER_BEAM_H
#define GAME_SERVER_ENTITIES_DRAGGER_BEAM_H

#include <game/server/entity.h>

class CDragger;
class CGameWorld;

 
class CDraggerBeam : public CEntity
{
	CDragger *m_pDragger;
	float m_Strength;
	bool m_IgnoreWalls;
	int m_ForClientId;
	int m_EvalTick;
	bool m_Active;

public:
	CDraggerBeam(CGameWorld *pGameWorld, CDragger *pDragger, vec2 Pos, float Strength, bool IgnoreWalls, int ForClientId, int Layer, int Number);

	void SetPos(vec2 Pos);

	void Reset() override;
	void Tick() override;
	void Snap(int SnappingClient) override;
	void SwapClients(int Client1, int Client2) override;
	ESaveResult BlocksSave(int ClientId) override;
};

#endif  
