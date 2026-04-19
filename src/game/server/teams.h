 
#ifndef GAME_SERVER_TEAMS_H
#define GAME_SERVER_TEAMS_H

#include <engine/shared/protocol.h>

#include <game/race_state.h>
#include <game/server/gamecontext.h>
#include <game/team_state.h>
#include <game/teamscore.h>

#include <memory>
#include <optional>

class CCharacter;
class CPlayer;
struct CScoreSaveResult;

class CGameTeams
{
	 
	 
	 
	 
	 
	 
	 
	bool m_aTeeStarted[MAX_CLIENTS];
	bool m_aTeeFinished[MAX_CLIENTS];
	int m_aLastChat[MAX_CLIENTS];

	ETeamState m_aTeamState[NUM_DDRACE_TEAMS];
	bool m_aTeamLocked[NUM_DDRACE_TEAMS];
	bool m_aTeamFlock[NUM_DDRACE_TEAMS];
	CClientMask m_aInvited[NUM_DDRACE_TEAMS];
	bool m_aPractice[NUM_DDRACE_TEAMS];
	std::shared_ptr<CScoreSaveResult> m_apSaveTeamResult[NUM_DDRACE_TEAMS];
	uint64_t m_aLastSwap[MAX_CLIENTS];  
	bool m_aTeamSentStartWarning[NUM_DDRACE_TEAMS];
	 
	 
	 
	 
	int m_aTeamUnfinishableKillTick[NUM_DDRACE_TEAMS];

	CGameContext *m_pGameContext;

	 
	void KillTeam(int Team, int NewStrongId, int ExceptId = -1);
	bool TeamFinished(int Team);
	void OnTeamFinish(int Team, CPlayer **Players, unsigned int Size, int TimeTicks, const char *pTimestamp);
	void OnFinish(CPlayer *Player, int TimeTicks, const char *pTimestamp);

public:
	CTeamsCore m_Core;

	CGameTeams(CGameContext *pGameContext);

	 
	CCharacter *Character(int ClientId);
	const CCharacter *Character(int ClientId) const;
	CPlayer *GetPlayer(int ClientId);
	CGameContext *GameServer();
	const CGameContext *GameServer() const;
	class IServer *Server();

	void OnCharacterStart(int ClientId);
	void OnCharacterFinish(int ClientId);
	void OnCharacterSpawn(int ClientId);
	void OnCharacterDeath(int ClientId, int Weapon);
	void Tick();

	 
	 
	bool CanJoinTeam(int ClientId, int Team, char *pError, int ErrorSize) const;

	 
	bool SetCharacterTeam(int ClientId, int Team, char *pError, int ErrorSize);
	void CheckTeamFinished(int Team);

	void ChangeTeamState(int Team, ETeamState State);

	CClientMask TeamMask(int Team, int ExceptId = -1, int Asker = -1, int VersionFlags = CGameContext::FLAG_SIX | CGameContext::FLAG_SIXUP);

	int Count(int Team) const;

	 
	void SetForceCharacterTeam(int ClientId, int Team);

	void Reset();
	void ResetRoundState(int Team);
	void ResetSwitchers(int Team);

	void SendTeamsState(int ClientId);
	void SetTeamLock(int Team, bool Lock);
	void SetTeamFlock(int Team, bool Mode);
	void ResetInvited(int Team);
	void SetClientInvited(int Team, int ClientId, bool Invited);

	ERaceState GetDDRaceState(const CPlayer *Player) const;
	int GetStartTime(CPlayer *Player);
	float *GetCurrentTimeCp(CPlayer *Player);
	void SetDDRaceState(CPlayer *Player, ERaceState DDRaceState);
	void SetStartTime(CPlayer *Player, int StartTime);
	void SetLastTimeCp(CPlayer *Player, int LastTimeCp);
	void KillCharacterOrTeam(int ClientId, int Team);
	void ResetSavedTeam(int ClientId, int Team);
	void RequestTeamSwap(CPlayer *pPlayer, CPlayer *pTargetPlayer, int Team);
	void SwapTeamCharacters(CPlayer *pPrimaryPlayer, CPlayer *pTargetPlayer, int Team);
	void CancelTeamSwap(CPlayer *pPlayer, int Team);
	void ProcessSaveTeam();
	std::optional<int> GetFirstEmptyTeam() const;
	bool TeeStarted(int ClientId) const;
	bool TeeFinished(int ClientId) const;
	ETeamState GetTeamState(int Team) const;
	bool TeamLocked(int Team) const;
	bool TeamFlock(int Team) const;
	bool IsInvited(int Team, int ClientId) const;
	bool IsStarted(int Team) const;
	void SetStarted(int ClientId, bool Started);
	void SetFinished(int ClientId, bool Finished);
	void SetSaving(int TeamId, std::shared_ptr<CScoreSaveResult> &SaveResult);
	bool GetSaving(int TeamId) const;
	void SetPractice(int Team, bool Enabled);
	bool IsPractice(int Team);
	bool IsValidTeamNumber(int Team) const;
};

#endif
