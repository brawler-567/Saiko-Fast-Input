#include "snapshot.h"

#include <base/system.h>

void CSnapshotBuilder::Init7(const CSnapshot *pSnapshot)
{
	 
	 
	m_Sixup = false;

	m_DataSize = pSnapshot->m_DataSize;
	m_NumItems = pSnapshot->m_NumItems;
	mem_copy(m_aOffsets, pSnapshot->Offsets(), sizeof(int) * m_NumItems);
	mem_copy(m_aData, pSnapshot->DataStart(), m_DataSize);
}
