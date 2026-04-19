#include "freezebars.h"

#include <game/client/gameclient.h>

void CFreezeBars::RenderFreezeBar(const int ClientId)
{
	const float FreezeBarWidth = 64.0f;
	const float FreezeBarHalfWidth = 32.0f;
	const float FreezeBarHeight = 16.0f;

	 
	CCharacterCore *pCharacter = &GameClient()->m_aClients[ClientId].m_Predicted;

	if(pCharacter->m_FreezeEnd <= 0 || pCharacter->m_FreezeStart == 0 || pCharacter->m_FreezeEnd <= pCharacter->m_FreezeStart || !GameClient()->m_Snap.m_aCharacters[ClientId].m_HasExtendedDisplayInfo || (pCharacter->m_IsInFreeze && g_Config.m_ClFreezeBarsAlphaInsideFreeze == 0))
	{
		return;
	}

	const int Max = pCharacter->m_FreezeEnd - pCharacter->m_FreezeStart;
	float FreezeProgress = std::clamp(Max - (Client()->GameTick(g_Config.m_ClDummy) - pCharacter->m_FreezeStart), 0, Max) / (float)Max;
	if(FreezeProgress <= 0.0f)
	{
		return;
	}

	vec2 Position = GameClient()->m_aClients[ClientId].m_RenderPos;
	Position.x -= FreezeBarHalfWidth;
	Position.y += 32;

	float Alpha = GameClient()->IsOtherTeam(ClientId) ? g_Config.m_ClShowOthersAlpha / 100.0f : 1.0f;
	if(pCharacter->m_IsInFreeze)
	{
		Alpha *= g_Config.m_ClFreezeBarsAlphaInsideFreeze / 100.0f;
	}

	RenderFreezeBarPos(Position.x, Position.y, FreezeBarWidth, FreezeBarHeight, FreezeProgress, Alpha);
}

void CFreezeBars::RenderFreezeBarPos(float x, const float y, const float Width, const float Height, float Progress, const float Alpha)
{
	Progress = std::clamp(Progress, 0.0f, 1.0f);

	 
	 
	const float RestPct = 0.5f;
	const float ProgPct = 0.5f;

	const float EndWidth = Height;  
	const float BarHeight = Height;
	const float WholeBarWidth = Width;
	const float MiddleBarWidth = WholeBarWidth - (EndWidth * 2.0f);
	const float EndProgressWidth = EndWidth * ProgPct;
	const float EndRestWidth = EndWidth * RestPct;
	const float ProgressBarWidth = WholeBarWidth - (EndProgressWidth * 2.0f);
	const float EndProgressProportion = EndProgressWidth / ProgressBarWidth;
	const float MiddleProgressProportion = MiddleBarWidth / ProgressBarWidth;

	 
	float BeginningPieceProgress = 1;
	if(Progress <= EndProgressProportion)
	{
		BeginningPieceProgress = Progress / EndProgressProportion;
	}

	 
	Graphics()->WrapClamp();
	Graphics()->TextureSet(GameClient()->m_HudSkin.m_SpriteHudFreezeBarFullLeft);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1.f, 1.f, 1.f, Alpha);
	 
	Graphics()->QuadsSetSubsetFree(0, 0, RestPct + ProgPct * BeginningPieceProgress, 0, RestPct + ProgPct * BeginningPieceProgress, 1, 0, 1);
	IGraphics::CQuadItem QuadFullBeginning(x, y, EndRestWidth + EndProgressWidth * BeginningPieceProgress, BarHeight);
	Graphics()->QuadsDrawTL(&QuadFullBeginning, 1);
	Graphics()->QuadsEnd();

	 
	if(BeginningPieceProgress < 1.0f)
	{
		Graphics()->TextureSet(GameClient()->m_HudSkin.m_SpriteHudFreezeBarEmptyRight);
		Graphics()->QuadsBegin();
		Graphics()->SetColor(1.f, 1.f, 1.f, Alpha);
		 
		Graphics()->QuadsSetSubsetFree(ProgPct - ProgPct * BeginningPieceProgress, 0, 0, 0, 0, 1, ProgPct - ProgPct * BeginningPieceProgress, 1);
		IGraphics::CQuadItem QuadEmptyBeginning(x + EndRestWidth + EndProgressWidth * BeginningPieceProgress, y, EndProgressWidth * (1.0f - BeginningPieceProgress), BarHeight);
		Graphics()->QuadsDrawTL(&QuadEmptyBeginning, 1);
		Graphics()->QuadsEnd();
	}

	 
	x += EndWidth;

	float MiddlePieceProgress = 1;
	if(Progress <= EndProgressProportion + MiddleProgressProportion)
	{
		if(Progress <= EndProgressProportion)
		{
			MiddlePieceProgress = 0;
		}
		else
		{
			MiddlePieceProgress = (Progress - EndProgressProportion) / MiddleProgressProportion;
		}
	}

	const float FullMiddleBarWidth = MiddleBarWidth * MiddlePieceProgress;
	const float EmptyMiddleBarWidth = MiddleBarWidth - FullMiddleBarWidth;

	 
	Graphics()->TextureSet(GameClient()->m_HudSkin.m_SpriteHudFreezeBarFull);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1.f, 1.f, 1.f, Alpha);
	 
	if(FullMiddleBarWidth <= EndWidth)
	{
		 
		 
		Graphics()->QuadsSetSubsetFree(0, 0, FullMiddleBarWidth / EndWidth, 0, FullMiddleBarWidth / EndWidth, 1, 0, 1);
	}
	else
	{
		 
		Graphics()->QuadsSetSubsetFree(0, 0, 1, 0, 1, 1, 0, 1);
	}
	IGraphics::CQuadItem QuadFull(x, y, FullMiddleBarWidth, BarHeight);
	Graphics()->QuadsDrawTL(&QuadFull, 1);
	Graphics()->QuadsEnd();

	 
	Graphics()->TextureSet(GameClient()->m_HudSkin.m_SpriteHudFreezeBarEmpty);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1.f, 1.f, 1.f, Alpha);
	 
	if(EmptyMiddleBarWidth <= EndWidth)
	{
		 
		 
		Graphics()->QuadsSetSubsetFree(EmptyMiddleBarWidth / EndWidth, 0, 0, 0, 0, 1, EmptyMiddleBarWidth / EndWidth, 1);
	}
	else
	{
		 
		Graphics()->QuadsSetSubsetFree(1, 0, 0, 0, 0, 1, 1, 1);
	}

	IGraphics::CQuadItem QuadEmpty(x + FullMiddleBarWidth, y, EmptyMiddleBarWidth, BarHeight);
	Graphics()->QuadsDrawTL(&QuadEmpty, 1);
	Graphics()->QuadsEnd();

	 
	x += MiddleBarWidth;
	float EndingPieceProgress = 1;
	if(Progress <= 1)
	{
		if(Progress <= (EndProgressProportion + MiddleProgressProportion))
		{
			EndingPieceProgress = 0;
		}
		else
		{
			EndingPieceProgress = (Progress - EndProgressProportion - MiddleProgressProportion) / EndProgressProportion;
		}
	}
	if(EndingPieceProgress > 0.0f)
	{
		 
		Graphics()->TextureSet(GameClient()->m_HudSkin.m_SpriteHudFreezeBarFullLeft);
		Graphics()->QuadsBegin();
		Graphics()->SetColor(1.f, 1.f, 1.f, Alpha);
		 
		Graphics()->QuadsSetSubsetFree(1, 0, 1.0f - ProgPct * EndingPieceProgress, 0, 1.0f - ProgPct * EndingPieceProgress, 1, 1, 1);
		IGraphics::CQuadItem QuadFullEnding(x, y, EndProgressWidth * EndingPieceProgress, BarHeight);
		Graphics()->QuadsDrawTL(&QuadFullEnding, 1);
		Graphics()->QuadsEnd();
	}
	 
	Graphics()->TextureSet(GameClient()->m_HudSkin.m_SpriteHudFreezeBarEmptyRight);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1.f, 1.f, 1.f, Alpha);
	 
	Graphics()->QuadsSetSubsetFree(ProgPct - ProgPct * (1.0f - EndingPieceProgress), 0, 1, 0, 1, 1, ProgPct - ProgPct * (1.0f - EndingPieceProgress), 1);
	IGraphics::CQuadItem QuadEmptyEnding(x + EndProgressWidth * EndingPieceProgress, y, EndProgressWidth * (1.0f - EndingPieceProgress) + EndRestWidth, BarHeight);
	Graphics()->QuadsDrawTL(&QuadEmptyEnding, 1);
	Graphics()->QuadsEnd();

	Graphics()->QuadsSetSubset(0, 0, 1, 1);
	Graphics()->SetColor(1.f, 1.f, 1.f, 1.f);
	Graphics()->WrapNormal();
}

inline bool CFreezeBars::IsPlayerInfoAvailable(int ClientId) const
{
	return GameClient()->m_Snap.m_aCharacters[ClientId].m_Active &&
	       GameClient()->m_Snap.m_apPrevPlayerInfos[ClientId] != nullptr &&
	       GameClient()->m_Snap.m_apPlayerInfos[ClientId] != nullptr;
}

void CFreezeBars::OnRender()
{
	if(Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;

	if(!g_Config.m_ClShowFreezeBars)
	{
		return;
	}
	 
	float ScreenX0, ScreenY0, ScreenX1, ScreenY1;
	Graphics()->GetScreen(&ScreenX0, &ScreenY0, &ScreenX1, &ScreenY1);
	 
	 
	 
	 
	float BorderBuffer = 120;
	ScreenX0 -= BorderBuffer;
	ScreenX1 += BorderBuffer;
	ScreenY0 -= BorderBuffer;
	ScreenY1 += BorderBuffer;

	int LocalClientId = GameClient()->m_Snap.m_LocalClientId;

	 
	for(int ClientId = 0; ClientId < MAX_CLIENTS; ClientId++)
	{
		if(ClientId == LocalClientId || !IsPlayerInfoAvailable(ClientId))
		{
			continue;
		}

		 
		vec2 *pRenderPos = &GameClient()->m_aClients[ClientId].m_RenderPos;
		if(pRenderPos->x < ScreenX0 || pRenderPos->x > ScreenX1 || pRenderPos->y < ScreenY0 || pRenderPos->y > ScreenY1)
		{
			continue;
		}

		RenderFreezeBar(ClientId);
	}
	if(LocalClientId != -1 && IsPlayerInfoAvailable(LocalClientId))
	{
		RenderFreezeBar(LocalClientId);
	}
}
