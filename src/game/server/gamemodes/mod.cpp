#include "mod.h"

#include <engine/shared/config.h>

 
 
 
#define GAME_TYPE_NAME "Mod"
#define TEST_TYPE_NAME "TestMod"

CGameControllerMod::CGameControllerMod(class CGameContext *pGameServer) :
	IGameController(pGameServer)
{
	m_pGameType = g_Config.m_SvTestingCommands ? TEST_TYPE_NAME : GAME_TYPE_NAME;

	 
}

CGameControllerMod::~CGameControllerMod() = default;

void CGameControllerMod::Tick()
{
	 

	IGameController::Tick();
}
