#include <engine/client/client.h>

int CClient::TranslateSysMsg(int *pMsgId, bool System, CUnpacker *pUnpacker, CPacker *pPacker, CNetChunk *pPacket, bool *pIsExMsg)
{
	*pIsExMsg = false;
	if(!System)
		return -1;

	 
	if(*pMsgId > NETMSG_WHATIS && *pMsgId < NETMSG_RCON_CMD_GROUP_END)
	{
		*pIsExMsg = true;
		return 0;
	}

	pPacker->Reset();

	if(*pMsgId == protocol7::NETMSG_MAP_CHANGE)
	{
		*pMsgId = NETMSG_MAP_CHANGE;
		const char *pMapName = pUnpacker->GetString(CUnpacker::SANITIZE_CC | CUnpacker::SKIP_START_WHITESPACES);
		int MapCrc = pUnpacker->GetInt();
		int Size = pUnpacker->GetInt();
		m_TranslationContext.m_MapDownloadChunksPerRequest = pUnpacker->GetInt();
		int ChunkSize = pUnpacker->GetInt();
		 
		pPacker->AddString(pMapName, 0);
		pPacker->AddInt(MapCrc);
		pPacker->AddInt(Size);
		m_TranslationContext.m_MapdownloadTotalsize = Size;
		m_TranslationContext.m_MapDownloadChunkSize = ChunkSize;
		return 0;
	}
	else if(*pMsgId == protocol7::NETMSG_SERVERINFO)
	{
		 
		 
		*pMsgId = -1;
		net_addr_str(&pPacket->m_Address, m_CurrentServerInfo.m_aAddress, sizeof(m_CurrentServerInfo.m_aAddress), true);
		str_copy(m_CurrentServerInfo.m_aVersion, pUnpacker->GetString(CUnpacker::SANITIZE_CC | CUnpacker::SKIP_START_WHITESPACES));
		str_copy(m_CurrentServerInfo.m_aName, pUnpacker->GetString(CUnpacker::SANITIZE_CC | CUnpacker::SKIP_START_WHITESPACES));
		str_clean_whitespaces(m_CurrentServerInfo.m_aName);
		pUnpacker->GetString(CUnpacker::SANITIZE_CC | CUnpacker::SKIP_START_WHITESPACES);  
		str_copy(m_CurrentServerInfo.m_aMap, pUnpacker->GetString(CUnpacker::SANITIZE_CC | CUnpacker::SKIP_START_WHITESPACES));
		str_copy(m_CurrentServerInfo.m_aGameType, pUnpacker->GetString(CUnpacker::SANITIZE_CC | CUnpacker::SKIP_START_WHITESPACES));
		int Flags = pUnpacker->GetInt();
		if(Flags & SERVER_FLAG_PASSWORD)
			m_CurrentServerInfo.m_Flags |= SERVER_FLAG_PASSWORD;
		 
		 
		 
		pUnpacker->GetInt();  
		m_CurrentServerInfo.m_NumPlayers = pUnpacker->GetInt();
		m_CurrentServerInfo.m_MaxPlayers = pUnpacker->GetInt();
		m_CurrentServerInfo.m_NumClients = pUnpacker->GetInt();
		m_CurrentServerInfo.m_MaxClients = pUnpacker->GetInt();
		return 0;
	}
	else if(*pMsgId == protocol7::NETMSG_RCON_AUTH_ON)
	{
		*pMsgId = NETMSG_RCON_AUTH_STATUS;
		pPacker->AddInt(1);  
		pPacker->AddInt(1);  
		return 0;
	}
	else if(*pMsgId == protocol7::NETMSG_RCON_AUTH_OFF)
	{
		*pMsgId = NETMSG_RCON_AUTH_STATUS;
		pPacker->AddInt(0);  
		pPacker->AddInt(0);  
		return 0;
	}
	else if(*pMsgId == protocol7::NETMSG_MAP_DATA)
	{
		 
		*pMsgId = NETMSG_MAP_DATA;
	}
	else if(*pMsgId >= protocol7::NETMSG_CON_READY && *pMsgId <= protocol7::NETMSG_INPUTTIMING)
	{
		*pMsgId = *pMsgId - 1;
	}
	else if(*pMsgId == protocol7::NETMSG_RCON_LINE)
	{
		*pMsgId = NETMSG_RCON_LINE;
	}
	else if(*pMsgId == protocol7::NETMSG_RCON_CMD_ADD)
	{
		*pMsgId = NETMSG_RCON_CMD_ADD;
	}
	else if(*pMsgId == protocol7::NETMSG_RCON_CMD_REM)
	{
		*pMsgId = NETMSG_RCON_CMD_REM;
	}
	else if(*pMsgId == protocol7::NETMSG_PING_REPLY)
	{
		*pMsgId = NETMSG_PING_REPLY;
	}
	else if(*pMsgId == protocol7::NETMSG_MAPLIST_ENTRY_ADD || *pMsgId == protocol7::NETMSG_MAPLIST_ENTRY_REM)
	{
		 
		return -1;
	}
	else if(*pMsgId >= NETMSG_INFO && *pMsgId <= NETMSG_MAP_DATA)
	{
		return -1;  
	}
	else if(*pMsgId < OFFSET_UUID)
	{
		dbg_msg("sixup", "drop unknown sys msg=%d", *pMsgId);
		return -1;
	}

	return -1;
}
