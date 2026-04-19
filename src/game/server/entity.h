 
 
#ifndef GAME_SERVER_ENTITY_H
#define GAME_SERVER_ENTITY_H

#include "gameworld.h"
#include "save.h"

#include <base/vmath.h>

#include <game/alloc.h>

class CCollision;
class CGameContext;

 
class CEntity
{
	MACRO_ALLOC_HEAP()

private:
	friend CGameWorld;  
	CEntity *m_pPrevTypeEntity;
	CEntity *m_pNextTypeEntity;

	 
	CGameWorld *m_pGameWorld;
	CCollision *m_pCCollision;

	int m_Id;
	int m_ObjType;

	 
	float m_ProximityRadius;

protected:
	 
	bool m_MarkedForDestroy;

public:  
	 
	vec2 m_Pos;

	 
	int GetId() const { return m_Id; }

	 
	CEntity(CGameWorld *pGameWorld, int Objtype, vec2 Pos = vec2(0, 0), int ProximityRadius = 0);

	 
	virtual ~CEntity();

	 
	std::vector<SSwitchers> &Switchers() { return m_pGameWorld->m_Core.m_vSwitchers; }
	CGameWorld *GameWorld() { return m_pGameWorld; }
	CTuningParams *GlobalTuning() { return &GameWorld()->TuningList()[0]; }
	CTuningParams *TuningList() { return GameWorld()->TuningList(); }
	CTuningParams *GetTuning(int i) { return GameWorld()->GetTuning(i); }
	class CConfig *Config() { return m_pGameWorld->Config(); }
	class CGameContext *GameServer() { return m_pGameWorld->GameServer(); }
	class IServer *Server() { return m_pGameWorld->Server(); }
	CCollision *Collision() { return m_pCCollision; }

	 
	CEntity *TypeNext() { return m_pNextTypeEntity; }
	CEntity *TypePrev() { return m_pPrevTypeEntity; }
	const vec2 &GetPos() const { return m_Pos; }
	float GetProximityRadius() const { return m_ProximityRadius; }

	 

	 
	virtual void Destroy() { delete this; }

	 
	virtual void Reset() {}

	 
	virtual void Tick() {}

	 
	virtual void TickDeferred() {}

	 
	virtual void TickPaused() {}

	 
	virtual void Snap(int SnappingClient) {}

	 
	virtual void SwapClients(int Client1, int Client2) {}

	 
	virtual ESaveResult BlocksSave(int ClientId) { return ESaveResult::SUCCESS; }

	 
	virtual int GetOwnerId() const { return -1; }

	 
	bool NetworkClipped(int SnappingClient) const;
	bool NetworkClipped(int SnappingClient, vec2 CheckPos) const;
	bool NetworkClippedLine(int SnappingClient, vec2 StartPos, vec2 EndPos) const;

	bool GameLayerClipped(vec2 CheckPos);
	virtual bool CanCollide(int ClientId) { return true; }

	 

	bool GetNearestAirPos(vec2 Pos, vec2 PrevPos, vec2 *pOutPos);
	bool GetNearestAirPosPlayer(vec2 PlayerPos, vec2 *pOutPos);

	int m_Number;
	int m_Layer;
};

bool NetworkClipped(const CGameContext *pGameServer, int SnappingClient, vec2 CheckPos);
bool NetworkClippedLine(const CGameContext *pGameServer, int SnappingClient, vec2 StartPos, vec2 EndPos);

#endif
