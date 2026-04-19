 
 
#ifndef GAME_SERVER_GAMEWORLD_H
#define GAME_SERVER_GAMEWORLD_H

#include "save.h"

#include <game/gamecore.h>

#include <vector>

class CCollision;
class CEntity;
class CCharacter;

 
class CGameWorld
{
public:
	enum
	{
		ENTTYPE_PROJECTILE = 0,
		ENTTYPE_LASER,
		ENTTYPE_PICKUP,
		ENTTYPE_FLAG,
		ENTTYPE_CHARACTER,
		NUM_ENTTYPES
	};

private:
	void Reset();
	void RemoveEntities();

	CEntity *m_pNextTraverseEntity = nullptr;
	CEntity *m_apFirstEntityTypes[NUM_ENTTYPES];

	class CGameContext *m_pGameServer;
	class CConfig *m_pConfig;
	class IServer *m_pServer;
	CTuningParams *m_pTuningList;

public:
	class CGameContext *GameServer() { return m_pGameServer; }
	class CConfig *Config() { return m_pConfig; }
	class IServer *Server() { return m_pServer; }

	bool m_ResetRequested;
	bool m_Paused;
	CWorldCore m_Core;

	CGameWorld();
	~CGameWorld();

	void SetGameServer(CGameContext *pGameServer);
	void Init(CCollision *pCollision, CTuningParams *pTuningList);

	CEntity *FindFirst(int Type);

	 
	int FindEntities(vec2 Pos, float Radius, CEntity **ppEnts, int Max, int Type);

	 
	CCharacter *IntersectCharacter(vec2 Pos0, vec2 Pos1, float Radius, vec2 &NewPos, const CCharacter *pNotThis = nullptr, int CollideWith = -1, const CCharacter *pThisOnly = nullptr);

	 
	CEntity *IntersectEntity(vec2 Pos0, vec2 Pos1, float Radius, int Type, vec2 &NewPos, const CEntity *pNotThis = nullptr, int CollideWith = -1, const CEntity *pThisOnly = nullptr);

	 
	CCharacter *ClosestCharacter(vec2 Pos, float Radius, const CEntity *pNotThis);

	 
	void InsertEntity(CEntity *pEntity);

	 
	void RemoveEntity(CEntity *pEntity);

	void RemoveEntitiesFromPlayer(int PlayerId);
	void RemoveEntitiesFromPlayers(int PlayerIds[], int NumPlayers);

	 
	void Snap(int SnappingClient);

	 
	void Tick();

	 
	void SwapClients(int Client1, int Client2);

	 
	ESaveResult BlocksSave(int ClientId);

	 
	void ReleaseHooked(int ClientId);

	 
	std::vector<CCharacter *> IntersectedCharacters(vec2 Pos0, vec2 Pos1, float Radius, const CEntity *pNotThis = nullptr);

	const CTuningParams *TuningList() const { return m_pTuningList; }
	CTuningParams *TuningList() { return m_pTuningList; }
	const CTuningParams *GetTuning(int i) const { return &TuningList()[i]; }
	CTuningParams *GetTuning(int i) { return &TuningList()[i]; }
};

#endif
