 
#include "plasma.h"

#include "character.h"

#include <engine/server.h>

#include <generated/protocol.h>

#include <game/server/gamecontext.h>
#include <game/teamscore.h>

const float PLASMA_ACCEL = 1.1f;

CPlasma::CPlasma(CGameWorld *pGameWorld, vec2 Pos, vec2 Dir, bool Freeze,
	bool Explosive, int ForClientId) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER)
{
	m_Pos = Pos;
	m_Core = Dir;
	m_Freeze = Freeze;
	m_Explosive = Explosive;
	m_ForClientId = ForClientId;
	m_EvalTick = Server()->Tick();
	m_LifeTime = Server()->TickSpeed() * 1.5f;

	GameWorld()->InsertEntity(this);
}

void CPlasma::Tick()
{
	 
	if(m_LifeTime == 0)
	{
		Reset();
		return;
	}
	CCharacter *pTarget = GameServer()->GetPlayerChar(m_ForClientId);
	 
	if(!pTarget)
	{
		Reset();
		return;
	}
	m_LifeTime--;
	Move();
	HitCharacter(pTarget);
	 
	HitObstacle(pTarget);
}

void CPlasma::Move()
{
	m_Pos += m_Core;
	m_Core *= PLASMA_ACCEL;
}

bool CPlasma::HitCharacter(CCharacter *pTarget)
{
	vec2 IntersectPos;
	CCharacter *pHitPlayer = GameServer()->m_World.IntersectCharacter(
		m_Pos, m_Pos + m_Core, 0.0f, IntersectPos, nullptr, m_ForClientId);
	if(!pHitPlayer)
	{
		return false;
	}

	 
	if(pHitPlayer->Team() == TEAM_SUPER)
	{
		return false;
	}

	m_Freeze ? pHitPlayer->Freeze() : pHitPlayer->UnFreeze();
	if(m_Explosive)
	{
		 
		 
		GameServer()->CreateExplosion(
			m_Pos, m_ForClientId, WEAPON_GRENADE, true, pTarget->Team(), pTarget->TeamMask());
	}
	Reset();
	return true;
}

bool CPlasma::HitObstacle(CCharacter *pTarget)
{
	 
	int HasIntersection = GameServer()->Collision()->IntersectNoLaser(m_Pos, m_Pos + m_Core, nullptr, nullptr);
	if(HasIntersection)
	{
		if(m_Explosive)
		{
			 
			GameServer()->CreateExplosion(
				m_Pos, m_ForClientId, WEAPON_GRENADE, true, pTarget->Team(), pTarget->TeamMask());
		}
		Reset();
		return true;
	}
	return false;
}

void CPlasma::Reset()
{
	m_MarkedForDestroy = true;
}

void CPlasma::Snap(int SnappingClient)
{
	 
	CCharacter *pTarget = GameServer()->GetPlayerChar(m_ForClientId);
	if(!pTarget || !pTarget->CanSnapCharacter(SnappingClient))
	{
		return;
	}

	 
	if(NetworkClipped(SnappingClient))
		return;

	int SnappingClientVersion = GameServer()->GetClientVersion(SnappingClient);

	int Subtype = (m_Explosive ? 1 : 0) | (m_Freeze ? 2 : 0);
	GameServer()->SnapLaserObject(CSnapContext(SnappingClientVersion, Server()->IsSixup(SnappingClient), SnappingClient), GetId(),
		m_Pos, m_Pos, m_EvalTick, m_ForClientId, LASERTYPE_PLASMA, Subtype, m_Number);
}

void CPlasma::SwapClients(int Client1, int Client2)
{
	m_ForClientId = m_ForClientId == Client1 ? Client2 : (m_ForClientId == Client2 ? Client1 : m_ForClientId);
}
