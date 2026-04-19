#ifndef ENGINE_SERVER_REGISTER_H
#define ENGINE_SERVER_REGISTER_H

class CConfig;
class IConsole;
class IEngine;
class IHttp;
struct CNetChunk;

class IRegister
{
public:
	virtual ~IRegister() = default;

	virtual void Update() = 0;
	 
	 
	virtual void OnConfigChange() = 0;
	 
	 
	virtual bool OnPacket(const CNetChunk *pPacket) = 0;
	 
	virtual void OnNewInfo(const char *pInfo) = 0;
	virtual void OnShutdown() = 0;
};

IRegister *CreateRegister(CConfig *pConfig, IConsole *pConsole, IEngine *pEngine, IHttp *pHttp, int ServerPort, unsigned SixupSecurityToken);

#endif
