 
#include "dragger_beam.h"

#include "character.h"
#include "dragger.h"

#include <engine/server.h>
#include <engine/shared/config.h>

#include <generated/protocol.h>

#include <game/mapitems.h>
#include <game/server/gamecontext.h>
#include <game/server/save.h>

CDraggerBeam::CDraggerBeam(CGameWorld *pGameWorld, CDragger *pDragger, vec2 Pos, float Strength, bool IgnoreWalls,
	int ForClientId, int Layer, int Number) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER)
{
	m_pDragger = pDragger;
	m_Pos = Pos;
	m_Strength = Strength;
	m_IgnoreWalls = IgnoreWalls;
	m_ForClientId = ForClientId;
	m_Active = true;
	m_Layer = Layer;
	m_Number = Number;
	m_EvalTick = Server()->Tick();

	GameWorld()->InsertEntity(this);
}

void CDraggerBeam::Tick()
{
	if(!m_Active)
	{
		return;
	}

	 
	CCharacter *pTarget = GameServer()->GetPlayerChar(m_ForClientId);
	if(!pTarget)
	{
		Reset();
		return;
	}

	 
	 
	 
	 
	if(Server()->Tick() % (int)(Server()->TickSpeed() * 0.15f) == 0)
	{
		if(m_Layer == LAYER_SWITCH && m_Number > 0 &&
			!Switchers()[m_Number].m_aStatus[pTarget->Team()])
		{
			Reset();
			return;
		}
	}

	 
	int IsReachable =
		m_IgnoreWalls ?
			!GameServer()->Collision()->IntersectNoLaserNoWalls(m_Pos, pTarget->m_Pos, nullptr, nullptr) :
			!GameServer()->Collision()->IntersectNoLaser(m_Pos, pTarget->m_Pos, nullptr, nullptr);
	if(!IsReachable ||
		distance(pTarget->m_Pos, m_Pos) >= g_Config.m_SvDraggerRange || !pTarget->IsAlive())
	{
		Reset();
		return;
	}
	 
	else if(distance(pTarget->m_Pos, m_Pos) > 28)
	{
		pTarget->AddVelocity(normalize(m_Pos - pTarget->m_Pos) * m_Strength);
	}
}

void CDraggerBeam::SetPos(vec2 Pos)
{
	m_Pos = Pos;
}

void CDraggerBeam::Reset()
{
	m_MarkedForDestroy = true;
	m_Active = false;

	m_pDragger->RemoveDraggerBeam(m_ForClientId);
}

void CDraggerBeam::Snap(int SnappingClient)
{
	if(!m_Active)
	{
		return;
	}

	 
	CCharacter *pTarget = GameServer()->GetPlayerChar(m_ForClientId);
	if(!pTarget || !pTarget->CanSnapCharacter(SnappingClient))
	{
		return;
	}
	 
	vec2 TargetPos = vec2(pTarget->m_Pos.x, pTarget->m_Pos.y);
	if(distance(pTarget->m_Pos, m_Pos) >= g_Config.m_SvDraggerRange || NetworkClippedLine(SnappingClient, m_Pos, TargetPos))
	{
		return;
	}

	int Subtype = (m_IgnoreWalls ? 1 : 0) | (std::clamp(round_to_int(m_Strength - 1.f), 0, 2) << 1);

	int StartTick = m_EvalTick;
	if(StartTick < Server()->Tick() - 4)
	{
		StartTick = Server()->Tick() - 4;
	}
	else if(StartTick > Server()->Tick())
	{
		StartTick = Server()->Tick();
	}

	int SnappingClientVersion = GameServer()->GetClientVersion(SnappingClient);
	if(SnappingClientVersion >= VERSION_DDNET_ENTITY_NETOBJS)
	{
		StartTick = -1;
	}

	int SnapObjId = GetId();
	if(m_pDragger->WillDraggerBeamUseDraggerId(m_ForClientId, SnappingClient))
	{
		SnapObjId = m_pDragger->GetId();
	}

	GameServer()->SnapLaserObject(CSnapContext(SnappingClientVersion, Server()->IsSixup(SnappingClient), SnappingClient), SnapObjId,
		TargetPos, m_Pos, StartTick, m_ForClientId, LASERTYPE_DRAGGER, Subtype, m_Number);
}

void CDraggerBeam::SwapClients(int Client1, int Client2)
{
	m_ForClientId = m_ForClientId == Client1 ? Client2 : (m_ForClientId == Client2 ? Client1 : m_ForClientId);
}

ESaveResult CDraggerBeam::BlocksSave(int ClientId)
{
	return m_ForClientId == ClientId ? ESaveResult::DRAGGER_ACTIVE : ESaveResult::SUCCESS;
}
