 
#include "plasma.h"

#include "character.h"

#include <game/client/laser_data.h>
#include <game/collision.h>
#include <game/mapitems.h>

const float PLASMA_ACCEL = 1.1f;

CPlasma::CPlasma(CGameWorld *pGameWorld, int Id, const CLaserData *pData) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_PLASMA)
{
	m_Id = Id;

	m_Number = pData->m_SwitchNumber;
	m_Layer = m_Number > 0 ? LAYER_SWITCH : LAYER_GAME;
	m_LifeTime = (int)(GameWorld()->GameTickSpeed() * 1.5f);

	m_Explosive = false;
	m_Freeze = false;

	Read(pData);

	CCharacter *pTarget = GameWorld()->GetCharacterById(m_ForClientId);
	if(!pTarget)
	{
		Reset();
		return;
	}
	m_Core = normalize(pTarget->m_Pos - m_Pos);
}

bool CPlasma::Match(const CPlasma *pPlasma) const
{
	return pPlasma->m_EvalTick == m_EvalTick && pPlasma->m_Number == m_Number &&
	       pPlasma->m_Explosive == m_Explosive && pPlasma->m_Freeze == m_Freeze && pPlasma->m_ForClientId == m_ForClientId;
}

void CPlasma::Read(const CLaserData *pData)
{
	m_Pos = pData->m_From;
	m_EvalTick = pData->m_StartTick;
	m_ForClientId = pData->m_Owner;

	if(0 <= pData->m_Subtype && pData->m_Subtype < NUM_LASERGUNTYPES)
	{
		m_Explosive = (pData->m_Subtype & 1);
		m_Freeze = (pData->m_Subtype & 2);
	}
}

void CPlasma::Reset()
{
	m_MarkedForDestroy = true;
}

void CPlasma::Tick()
{
	 
	if(m_LifeTime == 0)
	{
		Reset();
		return;
	}
	CCharacter *pTarget = GameWorld()->GetCharacterById(m_ForClientId);
	 
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
	CCharacter *pHitPlayer = GameWorld()->IntersectCharacter(
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
		 
		 
		GameWorld()->CreateExplosion(
			m_Pos, m_ForClientId, WEAPON_GRENADE, true, pTarget->Team(), CClientMask().set());
	}
	Reset();
	return true;
}

bool CPlasma::HitObstacle(CCharacter *pTarget)
{
	 
	int HasIntersection = Collision()->IntersectNoLaser(m_Pos, m_Pos + m_Core, nullptr, nullptr);
	if(HasIntersection)
	{
		if(m_Explosive)
		{
			 
			GameWorld()->CreateExplosion(
				m_Pos, m_ForClientId, WEAPON_GRENADE, true, pTarget->Team(), CClientMask().set());
		}
		Reset();
		return true;
	}
	return false;
}
