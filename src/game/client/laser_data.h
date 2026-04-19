 
 
#ifndef GAME_CLIENT_LASER_DATA_H
#define GAME_CLIENT_LASER_DATA_H

#include <base/vmath.h>

struct CNetObj_Laser;
struct CNetObj_DDNetLaser;
struct CNetObj_EntityEx;

class CLaserData
{
public:
	vec2 m_From;
	vec2 m_To;
	int m_StartTick;
	bool m_ExtraInfo;
	 
	int m_Owner;
	int m_Type;
	int m_SwitchNumber;
	int m_Subtype;
	bool m_Predict;
	 
	int m_TuneZone;
};

CLaserData ExtractLaserInfo(int NetObjType, const void *pData, class CGameWorld *pGameWorld, const CNetObj_EntityEx *pEntEx);
CLaserData ExtractLaserInfoDDNet(const CNetObj_DDNetLaser *pLaser, class CGameWorld *pGameWorld);

#endif  
