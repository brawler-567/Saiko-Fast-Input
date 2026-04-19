 
#include "gun.h"

#include "character.h"
#include "plasma.h"

#include <engine/server.h>
#include <engine/shared/config.h>

#include <generated/protocol.h>

#include <game/mapitems.h>
#include <game/server/gamecontext.h>
#include <game/server/player.h>
#include <game/server/teams.h>

CGun::CGun(CGameWorld *pGameWorld, vec2 Pos, bool Freeze, bool Explosive, int Layer, int Number) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER)
{
	m_Core = vec2(0.0f, 0.0f);
	m_Pos = Pos;
	m_Freeze = Freeze;
	m_Explosive = Explosive;
	m_Layer = Layer;
	m_Number = Number;
	m_EvalTick = Server()->Tick();

	std::fill(std::begin(m_aLastFireTeam), std::end(m_aLastFireTeam), 0);
	std::fill(std::begin(m_aLastFireSolo), std::end(m_aLastFireSolo), 0);
	GameWorld()->InsertEntity(this);
}

void CGun::Tick()
{
	if(Server()->Tick() % (int)(Server()->TickSpeed() * 0.15f) == 0)
	{
		m_EvalTick = Server()->Tick();
		GameServer()->Collision()->MoverSpeed(m_Pos.x, m_Pos.y, &m_Core);
		m_Pos += m_Core;
	}
	if(g_Config.m_SvPlasmaPerSec > 0)
	{
		Fire();
	}
}

void CGun::Fire()
{
	 
	CEntity *apPlayersInRange[MAX_CLIENTS];
	std::fill(std::begin(apPlayersInRange), std::end(apPlayersInRange), nullptr);

	int NumPlayersInRange = GameServer()->m_World.FindEntities(m_Pos, g_Config.m_SvPlasmaRange,
		apPlayersInRange, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);

	 
	int aTargetIdInTeam[MAX_CLIENTS];
	bool aIsTarget[MAX_CLIENTS];
	int aMinDistInTeam[MAX_CLIENTS];
	std::fill(std::begin(aMinDistInTeam), std::end(aMinDistInTeam), 0);
	std::fill(std::begin(aIsTarget), std::end(aIsTarget), false);
	std::fill(std::begin(aTargetIdInTeam), std::end(aTargetIdInTeam), -1);

	for(int i = 0; i < NumPlayersInRange; i++)
	{
		CCharacter *pTarget = static_cast<CCharacter *>(apPlayersInRange[i]);
		const int &TargetTeam = pTarget->Team();
		 
		if(TargetTeam == TEAM_SUPER)
		{
			continue;
		}
		 
		if(m_Layer == LAYER_SWITCH && m_Number > 0 &&
			!Switchers()[m_Number].m_aStatus[TargetTeam])
		{
			continue;
		}

		 
		const int &TargetClientId = pTarget->GetPlayer()->GetCid();
		const bool &TargetIsSolo = pTarget->Teams()->m_Core.GetSolo(TargetClientId);
		if((TargetIsSolo &&
			   m_aLastFireSolo[TargetClientId] + Server()->TickSpeed() / g_Config.m_SvPlasmaPerSec > Server()->Tick()) ||
			(!TargetIsSolo &&
				m_aLastFireTeam[TargetTeam] + Server()->TickSpeed() / g_Config.m_SvPlasmaPerSec > Server()->Tick()))
		{
			continue;
		}

		 
		int IsReachable = !GameServer()->Collision()->IntersectLine(m_Pos, pTarget->m_Pos, nullptr, nullptr);
		if(IsReachable && pTarget->IsAlive())
		{
			 
			if(TargetIsSolo)
			{
				aIsTarget[TargetClientId] = true;
				m_aLastFireSolo[TargetClientId] = Server()->Tick();
			}
			else
			{
				int Distance = distance(pTarget->m_Pos, m_Pos);
				if(aMinDistInTeam[TargetTeam] == 0 || aMinDistInTeam[TargetTeam] > Distance)
				{
					aMinDistInTeam[TargetTeam] = Distance;
					aTargetIdInTeam[TargetTeam] = TargetClientId;
				}
			}
		}
	}

	 
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(aTargetIdInTeam[i] != -1)
		{
			aIsTarget[aTargetIdInTeam[i]] = true;
			m_aLastFireTeam[i] = Server()->Tick();
		}
	}

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		 
		if(aIsTarget[i])
		{
			CCharacter *pTarget = GameServer()->GetPlayerChar(i);
			new CPlasma(&GameServer()->m_World, m_Pos, normalize(pTarget->m_Pos - m_Pos), m_Freeze, m_Explosive, i);
		}
	}
}

void CGun::Reset()
{
	m_MarkedForDestroy = true;
}

void CGun::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	int SnappingClientVersion = GameServer()->GetClientVersion(SnappingClient);

	int Subtype = (m_Explosive ? 1 : 0) | (m_Freeze ? 2 : 0);

	int StartTick;
	if(SnappingClientVersion >= VERSION_DDNET_ENTITY_NETOBJS)
	{
		StartTick = -1;
	}
	else
	{
		 
		CCharacter *pChar = GameServer()->GetPlayerChar(SnappingClient);

		if(SnappingClient != SERVER_DEMO_CLIENT &&
			(GameServer()->m_apPlayers[SnappingClient]->GetTeam() == TEAM_SPECTATORS ||
				GameServer()->m_apPlayers[SnappingClient]->IsPaused()) &&
			GameServer()->m_apPlayers[SnappingClient]->SpectatorId() != SPEC_FREEVIEW)
			pChar = GameServer()->GetPlayerChar(GameServer()->m_apPlayers[SnappingClient]->SpectatorId());

		int Tick = (Server()->Tick() % Server()->TickSpeed()) % 11;
		if(pChar && m_Layer == LAYER_SWITCH && m_Number > 0 &&
			!Switchers()[m_Number].m_aStatus[pChar->Team()] && (!Tick))
			return;

		StartTick = m_EvalTick;
	}

	GameServer()->SnapLaserObject(CSnapContext(SnappingClientVersion, Server()->IsSixup(SnappingClient), SnappingClient), GetId(),
		m_Pos, m_Pos, StartTick, -1, LASERTYPE_GUN, Subtype, m_Number);
}
