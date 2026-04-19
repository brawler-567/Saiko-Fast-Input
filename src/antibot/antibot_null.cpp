#define ANTIBOTAPI DYNAMIC_EXPORT

#include <antibot/antibot_interface.h>

#include <cstring>

static CAntibotData *g_pData;

extern "C" {

int AntibotAbiVersion()
{
	return ANTIBOT_ABI_VERSION;
}
void AntibotInit(CAntibotData *pCallbackData)
{
	g_pData = pCallbackData;
	g_pData->m_pfnLog("null antibot initialized", g_pData->m_pUser);
}
void AntibotRoundStart(CAntibotRoundData *pRoundData) {}
void AntibotRoundEnd(void) {}
void AntibotUpdateData(void) {}
void AntibotDestroy(void) { g_pData = 0; }
void AntibotConsoleCommand(const char *pCommand)
{
	if(strcmp(pCommand, "dump") == 0)
	{
		g_pData->m_pfnLog("null antibot", g_pData->m_pUser);
	}
	else
	{
		g_pData->m_pfnLog("unknown command", g_pData->m_pUser);
	}
}
void AntibotOnPlayerInit(int  ) {}
void AntibotOnPlayerDestroy(int  ) {}
void AntibotOnSpawn(int  ) {}
void AntibotOnHammerFireReloading(int  ) {}
void AntibotOnHammerFire(int  ) {}
void AntibotOnHammerHit(int  , int  ) {}
void AntibotOnDirectInput(int  ) {}
void AntibotOnCharacterTick(int  ) {}
void AntibotOnHookAttach(int  , bool  ) {}
void AntibotOnEngineTick(void) {}
void AntibotOnEngineClientJoin(int  ) {}
void AntibotOnEngineClientDrop(int  , const char *  ) {}
bool AntibotOnEngineClientMessage(int  , const void *  , int  , int  ) { return false; }
bool AntibotOnEngineServerMessage(int  , const void *  , int  , int  ) { return false; }
bool AntibotOnEngineSimulateClientMessage(int *  , void *  , int  , int *  , int *  ) { return false; }
}
