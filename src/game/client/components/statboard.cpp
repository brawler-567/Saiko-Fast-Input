#include <engine/graphics.h>
#include <engine/serverbrowser.h>
#include <engine/shared/config.h>
#include <engine/storage.h>
#include <engine/textrender.h>

#include <generated/client_data.h>

#include <game/client/animstate.h>
#include <game/client/components/motd.h>
#include <game/client/components/statboard.h>
#include <game/client/gameclient.h>
#include <game/localization.h>

CStatboard::CStatboard()
{
	m_Active = false;
	m_ScreenshotTaken = false;
	m_ScreenshotTime = -1;
}

void CStatboard::OnReset()
{
	for(auto &Stat : GameClient()->m_aStats)
		Stat.Reset();
	m_Active = false;
	m_ScreenshotTaken = false;
	m_ScreenshotTime = -1;
}

void CStatboard::OnRelease()
{
	m_Active = false;
}

void CStatboard::ConKeyStats(IConsole::IResult *pResult, void *pUserData)
{
	((CStatboard *)pUserData)->m_Active = pResult->GetInteger(0) != 0;
}

void CStatboard::OnConsoleInit()
{
	Console()->Register("+statboard", "", CFGFLAG_CLIENT, ConKeyStats, this, "Show stats");
}

bool CStatboard::IsActive() const
{
	return m_Active;
}

void CStatboard::OnMessage(int MsgType, void *pRawMsg)
{
	if(GameClient()->m_SuppressEvents)
		return;

	if(MsgType == NETMSGTYPE_SV_KILLMSG)
	{
		CNetMsg_Sv_KillMsg *pMsg = (CNetMsg_Sv_KillMsg *)pRawMsg;
		CGameClient::CClientStats *pStats = GameClient()->m_aStats;

		pStats[pMsg->m_Victim].m_Deaths++;
		pStats[pMsg->m_Victim].m_CurrentSpree = 0;
		if(pMsg->m_Weapon >= 0)
			pStats[pMsg->m_Victim].m_aDeathsFrom[pMsg->m_Weapon]++;
		if(pMsg->m_Victim != pMsg->m_Killer)
		{
			pStats[pMsg->m_Killer].m_Frags++;
			pStats[pMsg->m_Killer].m_CurrentSpree++;

			if(pStats[pMsg->m_Killer].m_CurrentSpree > pStats[pMsg->m_Killer].m_BestSpree)
				pStats[pMsg->m_Killer].m_BestSpree = pStats[pMsg->m_Killer].m_CurrentSpree;
			if(pMsg->m_Weapon >= 0)
				pStats[pMsg->m_Killer].m_aFragsWith[pMsg->m_Weapon]++;
		}
		else
			pStats[pMsg->m_Victim].m_Suicides++;
	}
	else if(MsgType == NETMSGTYPE_SV_KILLMSGTEAM)
	{
		CNetMsg_Sv_KillMsgTeam *pMsg = (CNetMsg_Sv_KillMsgTeam *)pRawMsg;
		CGameClient::CClientStats *pStats = GameClient()->m_aStats;

		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(GameClient()->m_Teams.Team(i) == pMsg->m_Team)
			{
				pStats[i].m_Deaths++;
				pStats[i].m_Suicides++;
			}
		}
	}
	else if(MsgType == NETMSGTYPE_SV_CHAT)
	{
		CNetMsg_Sv_Chat *pMsg = (CNetMsg_Sv_Chat *)pRawMsg;
		if(pMsg->m_ClientId < 0)
		{
			const char *p, *t;
			const char *pLookFor = "flag was captured by '";
			if((p = str_find(pMsg->m_pMessage, pLookFor)))
			{
				char aName[MAX_NAME_LENGTH];
				p += str_length(pLookFor);
				t = str_rchr(p, '\'');

				if(t <= p)
					return;
				str_truncate(aName, sizeof(aName), p, t - p);

				for(int i = 0; i < MAX_CLIENTS; i++)
				{
					if(!GameClient()->m_aStats[i].IsActive())
						continue;

					if(str_comp(GameClient()->m_aClients[i].m_aName, aName) == 0)
					{
						GameClient()->m_aStats[i].m_FlagCaptures++;
						break;
					}
				}
			}
		}
	}
}

void CStatboard::OnRender()
{
	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;

	if((g_Config.m_ClAutoStatboardScreenshot || g_Config.m_ClAutoCSV) && Client()->State() != IClient::STATE_DEMOPLAYBACK)
	{
		if(m_ScreenshotTime < 0 && GameClient()->m_Snap.m_pGameInfoObj && GameClient()->m_Snap.m_pGameInfoObj->m_GameStateFlags & GAMESTATEFLAG_GAMEOVER)
			m_ScreenshotTime = time_get() + time_freq() * 3;
		if(m_ScreenshotTime > -1 && m_ScreenshotTime < time_get())
			m_Active = true;
		if(!m_ScreenshotTaken && m_ScreenshotTime > -1 && m_ScreenshotTime + time_freq() / 5 < time_get())
		{
			if(g_Config.m_ClAutoStatboardScreenshot)
				AutoStatScreenshot();
			if(g_Config.m_ClAutoCSV)
				AutoStatCSV();
			m_ScreenshotTaken = true;
		}
	}

	if(IsActive())
		RenderGlobalStats();
}

void CStatboard::RenderGlobalStats()
{
	const float StatboardWidth = 400 * 3.0f * Graphics()->ScreenAspect();
	const float StatboardHeight = 400 * 3.0f;
	float StatboardContentWidth = 260.0f;
	float StatboardContentHeight = 750.0f;

	const CNetObj_PlayerInfo *apPlayers[MAX_CLIENTS] = {nullptr};
	int NumPlayers = 0;

	 
	for(const auto *pInfo : GameClient()->m_Snap.m_apInfoByScore)
	{
		if(!pInfo || !GameClient()->m_aStats[pInfo->m_ClientId].IsActive() || GameClient()->m_aClients[pInfo->m_ClientId].m_Team != TEAM_RED)
			continue;
		apPlayers[NumPlayers] = pInfo;
		NumPlayers++;
	}

	 
	if(GameClient()->IsTeamPlay())
	{
		for(const auto *pInfo : GameClient()->m_Snap.m_apInfoByScore)
		{
			if(!pInfo || !GameClient()->m_aStats[pInfo->m_ClientId].IsActive() || GameClient()->m_aClients[pInfo->m_ClientId].m_Team != TEAM_BLUE)
				continue;
			apPlayers[NumPlayers] = pInfo;
			NumPlayers++;
		}
	}

	 
	 
	if(NumPlayers > 32)
		return;

	 
	if(GameClient()->m_Motd.IsActive())
		GameClient()->m_Motd.Clear();

	bool GameWithFlags = GameClient()->m_Snap.m_pGameInfoObj &&
			     GameClient()->m_Snap.m_pGameInfoObj->m_GameFlags & GAMEFLAG_FLAGS;

	StatboardContentWidth += 7 * 85 + 95;  

	if(GameWithFlags)
		StatboardContentWidth += 150;  

	bool aDisplayWeapon[NUM_WEAPONS] = {false};
	for(int i = 0; i < NumPlayers; i++)
	{
		const CGameClient::CClientStats *pStats = &GameClient()->m_aStats[apPlayers[i]->m_ClientId];
		for(int j = 0; j < NUM_WEAPONS; j++)
			aDisplayWeapon[j] = aDisplayWeapon[j] || pStats->m_aFragsWith[j] || pStats->m_aDeathsFrom[j];
	}
	for(bool DisplayWeapon : aDisplayWeapon)
		if(DisplayWeapon)
			StatboardContentWidth += 80;

	float x = StatboardWidth / 2 - StatboardContentWidth / 2;
	float y = 200.0f;

	Graphics()->MapScreen(0, 0, StatboardWidth, StatboardHeight);

	Graphics()->DrawRect(x - 10.f, y - 10.f, StatboardContentWidth, StatboardContentHeight, ColorRGBA(0.0f, 0.0f, 0.0f, 0.5f), IGraphics::CORNER_ALL, 17.0f);

	int px = 325;

	TextRender()->Text(x + 10, y - 5, 22.0f, Localize("Name"), -1.0f);
	const char *apHeaders[] = {
		Localize("Frags"), Localize("Deaths"), Localize("Suicides"),
		Localize("Ratio"), Localize("Net"), Localize("FPM"),
		Localize("Spree"), Localize("Best"), Localize("Grabs")};
	for(int i = 0; i < 9; i++)
	{
		if(i == 2)
			px += 10.0f;  
		if(i == 8 && !GameWithFlags)  
			continue;
		const float TextWidth = TextRender()->TextWidth(22.0f, apHeaders[i], -1, -1.0f);
		TextRender()->Text(x + px - TextWidth, y - 5, 22.0f, apHeaders[i], -1.0f);
		px += 85;
	}

	px -= 40;
	for(int i = 0; i < NUM_WEAPONS; i++)
	{
		if(!aDisplayWeapon[i])
			continue;
		float ScaleX, ScaleY;
		Graphics()->GetSpriteScale(g_pData->m_Weapons.m_aId[i].m_pSpriteBody, ScaleX, ScaleY);
		Graphics()->TextureSet(GameClient()->m_GameSkin.m_aSpriteWeapons[i]);
		Graphics()->QuadsBegin();
		if(i == 0)
			Graphics()->DrawSprite(x + px, y + 10, g_pData->m_Weapons.m_aId[i].m_VisualSize * 0.8f * ScaleX, g_pData->m_Weapons.m_aId[i].m_VisualSize * 0.8f * ScaleY);
		else
			Graphics()->DrawSprite(x + px, y + 10, g_pData->m_Weapons.m_aId[i].m_VisualSize * ScaleX, g_pData->m_Weapons.m_aId[i].m_VisualSize * ScaleY);
		px += 80;
		Graphics()->QuadsEnd();
	}

	if(GameWithFlags)
	{
		Graphics()->TextureSet(GameClient()->m_GameSkin.m_SpriteFlagRed);
		float ScaleX, ScaleY;
		Graphics()->GetSpriteScale(SPRITE_FLAG_RED, ScaleX, ScaleY);
		Graphics()->QuadsBegin();
		Graphics()->QuadsSetRotation(0.78f);
		Graphics()->DrawSprite(x + px, y + 15, 48 * ScaleX, 48 * ScaleY);
		Graphics()->QuadsEnd();
	}

	y += 29.0f;

	float FontSize = 24.0f;
	float LineHeight = 50.0f;
	float TeeSizemod = 0.8f;
	float ContentLineOffset = LineHeight * 0.05f;

	if(NumPlayers > 16)
	{
		FontSize = 20.0f;
		LineHeight = 22.0f;
		TeeSizemod = 0.34f;
		ContentLineOffset = 0;
	}
	else if(NumPlayers > 14)
	{
		FontSize = 24.0f;
		LineHeight = 40.0f;
		TeeSizemod = 0.7f;
	}

	for(int j = 0; j < NumPlayers; j++)
	{
		const CNetObj_PlayerInfo *pInfo = apPlayers[j];
		const CGameClient::CClientStats *pStats = &GameClient()->m_aStats[pInfo->m_ClientId];

		if(GameClient()->m_Snap.m_LocalClientId == pInfo->m_ClientId || (GameClient()->m_Snap.m_SpecInfo.m_Active && pInfo->m_ClientId == GameClient()->m_Snap.m_SpecInfo.m_SpectatorId))
		{
			 
			Graphics()->DrawRect(x - 10, y + ContentLineOffset / 2, StatboardContentWidth, LineHeight - ContentLineOffset, ColorRGBA(1.0f, 1.0f, 1.0f, 0.25f), IGraphics::CORNER_NONE, 0.0f);
		}

		CTeeRenderInfo Teeinfo = GameClient()->m_aClients[pInfo->m_ClientId].m_RenderInfo;
		Teeinfo.m_Size *= TeeSizemod;

		const CAnimState *pIdleState = CAnimState::GetIdle();
		vec2 OffsetToMid;
		CRenderTools::GetRenderTeeOffsetToRenderedTee(pIdleState, &Teeinfo, OffsetToMid);
		vec2 TeeRenderPos(x + Teeinfo.m_Size / 2, y + LineHeight / 2.0f + OffsetToMid.y);

		RenderTools()->RenderTee(pIdleState, &Teeinfo, EMOTE_NORMAL, vec2(1, 0), TeeRenderPos);

		char aBuf[128];
		CTextCursor Cursor;
		Cursor.SetPosition(vec2(x + 64, y + (LineHeight * 0.95f - FontSize) / 2.f));
		Cursor.m_FontSize = FontSize;
		Cursor.m_Flags |= TEXTFLAG_STOP_AT_END;
		Cursor.m_LineWidth = 220;
		TextRender()->TextEx(&Cursor, GameClient()->m_aClients[pInfo->m_ClientId].m_aName, -1);

		px = 325;

		 
		{
			str_format(aBuf, sizeof(aBuf), "%d", pStats->m_Frags);
			const float TextWidth = TextRender()->TextWidth(FontSize, aBuf, -1, -1.0f);
			TextRender()->Text(x - TextWidth + px, y + (LineHeight * 0.95f - FontSize) / 2.f, FontSize, aBuf, -1.0f);
			px += 85;
		}
		 
		{
			str_format(aBuf, sizeof(aBuf), "%d", pStats->m_Deaths);
			const float TextWidth = TextRender()->TextWidth(FontSize, aBuf, -1, -1.0f);
			TextRender()->Text(x - TextWidth + px, y + (LineHeight * 0.95f - FontSize) / 2.f, FontSize, aBuf, -1.0f);
			px += 85;
		}
		 
		{
			px += 10;
			str_format(aBuf, sizeof(aBuf), "%d", pStats->m_Suicides);
			const float TextWidth = TextRender()->TextWidth(FontSize, aBuf, -1, -1.0f);
			TextRender()->Text(x - TextWidth + px, y + (LineHeight * 0.95f - FontSize) / 2.f, FontSize, aBuf, -1.0f);
			px += 85;
		}
		 
		{
			if(pStats->m_Deaths == 0)
				str_copy(aBuf, "--");
			else
				str_format(aBuf, sizeof(aBuf), "%.2f", (float)(pStats->m_Frags) / pStats->m_Deaths);
			const float TextWidth = TextRender()->TextWidth(FontSize, aBuf, -1, -1.0f);
			TextRender()->Text(x - TextWidth + px, y + (LineHeight * 0.95f - FontSize) / 2.f, FontSize, aBuf, -1.0f);
			px += 85;
		}
		 
		{
			str_format(aBuf, sizeof(aBuf), "%+d", pStats->m_Frags - pStats->m_Deaths);
			const float TextWidth = TextRender()->TextWidth(FontSize, aBuf, -1, -1.0f);
			TextRender()->Text(x - TextWidth + px, y + (LineHeight * 0.95f - FontSize) / 2.f, FontSize, aBuf, -1.0f);
			px += 85;
		}
		 
		{
			float Fpm = pStats->GetFPM(Client()->GameTick(g_Config.m_ClDummy), Client()->GameTickSpeed());
			str_format(aBuf, sizeof(aBuf), "%.1f", Fpm);
			const float TextWidth = TextRender()->TextWidth(FontSize, aBuf, -1, -1.0f);
			TextRender()->Text(x - TextWidth + px, y + (LineHeight * 0.95f - FontSize) / 2.f, FontSize, aBuf, -1.0f);
			px += 85;
		}
		 
		{
			str_format(aBuf, sizeof(aBuf), "%d", pStats->m_CurrentSpree);
			const float TextWidth = TextRender()->TextWidth(FontSize, aBuf, -1, -1.0f);
			TextRender()->Text(x - TextWidth + px, y + (LineHeight * 0.95f - FontSize) / 2.f, FontSize, aBuf, -1.0f);
			px += 85;
		}
		 
		{
			str_format(aBuf, sizeof(aBuf), "%d", pStats->m_BestSpree);
			const float TextWidth = TextRender()->TextWidth(FontSize, aBuf, -1, -1.0f);
			TextRender()->Text(x - TextWidth + px, y + (LineHeight * 0.95f - FontSize) / 2.f, FontSize, aBuf, -1.0f);
			px += 85;
		}
		 
		if(GameWithFlags)
		{
			str_format(aBuf, sizeof(aBuf), "%d", pStats->m_FlagGrabs);
			const float TextWidth = TextRender()->TextWidth(FontSize, aBuf, -1, -1.0f);
			TextRender()->Text(x - TextWidth + px, y + (LineHeight * 0.95f - FontSize) / 2.f, FontSize, aBuf, -1.0f);
			px += 85;
		}
		 
		px -= 40;
		for(int i = 0; i < NUM_WEAPONS; i++)
		{
			if(!aDisplayWeapon[i])
				continue;

			str_format(aBuf, sizeof(aBuf), "%d/%d", pStats->m_aFragsWith[i], pStats->m_aDeathsFrom[i]);
			const float TextWidth = TextRender()->TextWidth(FontSize, aBuf, -1, -1.0f);
			TextRender()->Text(x + px - TextWidth / 2, y + (LineHeight * 0.95f - FontSize) / 2.f, FontSize, aBuf, -1.0f);
			px += 80;
		}
		 
		if(GameWithFlags)
		{
			str_format(aBuf, sizeof(aBuf), "%d", pStats->m_FlagCaptures);
			const float TextWidth = TextRender()->TextWidth(FontSize, aBuf, -1, -1.0f);
			TextRender()->Text(x - TextWidth + px, y + (LineHeight * 0.95f - FontSize) / 2.f, FontSize, aBuf, -1.0f);
		}
		y += LineHeight;
	}
}

void CStatboard::AutoStatScreenshot()
{
	if(Client()->State() != IClient::STATE_DEMOPLAYBACK)
		Client()->AutoStatScreenshot_Start();
}

void CStatboard::AutoStatCSV()
{
	if(Client()->State() != IClient::STATE_DEMOPLAYBACK)
	{
		char aDate[20], aFilename[IO_MAX_PATH_LENGTH];
		str_timestamp(aDate, sizeof(aDate));
		str_format(aFilename, sizeof(aFilename), "screenshots/auto/stats_%s.csv", aDate);
		IOHANDLE File = Storage()->OpenFile(aFilename, IOFLAG_WRITE, IStorage::TYPE_SAVE);

		if(File)
		{
			char aStats[1024 * (VANILLA_MAX_CLIENTS + 1)];
			FormatStats(aStats, sizeof(aStats));
			io_write(File, aStats, str_length(aStats));
			io_close(File);
		}

		Client()->AutoCSV_Start();
	}
}

std::string CStatboard::ReplaceCommata(char *pStr)
{
	if(!str_find(pStr, ","))
		return pStr;

	char aOutbuf[256] = "";
	for(int i = 0, Skip = 0; i < 64; i++)
	{
		if(pStr[i] == ',')
		{
			aOutbuf[i + Skip++] = '%';
			aOutbuf[i + Skip++] = '2';
			aOutbuf[i + Skip] = 'C';
		}
		else
			aOutbuf[i + Skip] = pStr[i];
	}
	return aOutbuf;
}

void CStatboard::FormatStats(char *pDest, size_t DestSize)
{
	 
	CServerInfo CurrentServerInfo;
	Client()->GetServerInfo(&CurrentServerInfo);
	char aServerStats[1024];
	str_format(aServerStats, sizeof(aServerStats), "Servername,Game-type,Map\n%s,%s,%s", ReplaceCommata(CurrentServerInfo.m_aName).c_str(), ReplaceCommata(CurrentServerInfo.m_aGameType).c_str(), ReplaceCommata(CurrentServerInfo.m_aMap).c_str());

	 

	 
	const CNetObj_PlayerInfo *apPlayers[MAX_CLIENTS] = {nullptr};
	int NumPlayers = 0;

	 
	for(const auto *pInfo : GameClient()->m_Snap.m_apInfoByScore)
	{
		if(!pInfo || !GameClient()->m_aStats[pInfo->m_ClientId].IsActive() || GameClient()->m_aClients[pInfo->m_ClientId].m_Team != TEAM_RED)
			continue;
		apPlayers[NumPlayers] = pInfo;
		NumPlayers++;
	}

	 
	if(GameClient()->IsTeamPlay())
	{
		for(const auto *pInfo : GameClient()->m_Snap.m_apInfoByScore)
		{
			if(!pInfo || !GameClient()->m_aStats[pInfo->m_ClientId].IsActive() || GameClient()->m_aClients[pInfo->m_ClientId].m_Team != TEAM_BLUE)
				continue;
			apPlayers[NumPlayers] = pInfo;
			NumPlayers++;
		}
	}

	char aPlayerStats[1024 * VANILLA_MAX_CLIENTS] = "Local-player,Team,Name,Clan,Score,Frags,Deaths,Suicides,F/D-ratio,Net,FPM,Spree,Best,Hammer-F/D,Gun-F/D,Shotgun-F/D,Grenade-F/D,Laser-F/D,Ninja-F/D,GameWithFlags,Flag-grabs,Flag-captures\n";
	for(int i = 0; i < NumPlayers; i++)
	{
		const CNetObj_PlayerInfo *pInfo = apPlayers[i];
		const CGameClient::CClientStats *pStats = &GameClient()->m_aStats[pInfo->m_ClientId];

		 

		 
		char aWeaponFD[64 * NUM_WEAPONS];
		for(int j = 0; j < NUM_WEAPONS; j++)
		{
			if(j == 0)
				str_format(aWeaponFD, sizeof(aWeaponFD), "%d/%d", pStats->m_aFragsWith[j], pStats->m_aDeathsFrom[j]);
			else
				str_format(aWeaponFD, sizeof(aWeaponFD), "%s,%d/%d", aWeaponFD, pStats->m_aFragsWith[j], pStats->m_aDeathsFrom[j]);
		}

		 
		float KillRatio = 0.0f;
		if(pStats->m_Deaths != 0)
			KillRatio = (float)(pStats->m_Frags) / pStats->m_Deaths;

		 
		bool LocalPlayer = (GameClient()->m_Snap.m_LocalClientId == pInfo->m_ClientId || (GameClient()->m_Snap.m_SpecInfo.m_Active && pInfo->m_ClientId == GameClient()->m_Snap.m_SpecInfo.m_SpectatorId));

		 
		bool GameWithFlags = (GameClient()->m_Snap.m_pGameInfoObj && GameClient()->m_Snap.m_pGameInfoObj->m_GameFlags & GAMEFLAG_FLAGS);

		char aBuf[1024];
		str_format(aBuf, sizeof(aBuf), "%d,%d,%s,%s,%d,%d,%d,%d,%.2f,%i,%.1f,%d,%d,%s,%d,%d,%d\n",
			LocalPlayer ? 1 : 0,  
			GameClient()->m_aClients[pInfo->m_ClientId].m_Team,  
			ReplaceCommata(GameClient()->m_aClients[pInfo->m_ClientId].m_aName).c_str(),  
			ReplaceCommata(GameClient()->m_aClients[pInfo->m_ClientId].m_aClan).c_str(),  
			std::clamp(pInfo->m_Score, -999, 999),  
			pStats->m_Frags,  
			pStats->m_Deaths,  
			pStats->m_Suicides,  
			KillRatio,  
			pStats->m_Frags - pStats->m_Deaths,  
			pStats->GetFPM(Client()->GameTick(g_Config.m_ClDummy), Client()->GameTickSpeed()),  
			pStats->m_CurrentSpree,  
			pStats->m_BestSpree,  
			aWeaponFD,  
			GameWithFlags ? 1 : 0,  
			pStats->m_FlagGrabs,  
			pStats->m_FlagCaptures);  

		str_append(aPlayerStats, aBuf);
	}

	str_format(pDest, DestSize, "%s\n\n%s", aServerStats, aPlayerStats);
}
