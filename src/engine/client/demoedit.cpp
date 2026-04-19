#include "demoedit.h"

#include <engine/shared/demo.h>
#include <engine/storage.h>

CDemoEdit::CDemoEdit(const char *pNetVersion, class CSnapshotDelta *pSnapshotDelta, IStorage *pStorage, const char *pDemo, const char *pDst, int StartTick, int EndTick) :
	m_SnapshotDelta(*pSnapshotDelta),
	m_pStorage(pStorage)
{
	str_copy(m_aDemo, pDemo);
	str_copy(m_aDst, pDst);

	m_StartTick = StartTick;
	m_EndTick = EndTick;

	 
	m_DemoEditor.Init(&m_SnapshotDelta, nullptr, pStorage);
}

void CDemoEdit::Run()
{
	 
	m_Success = m_DemoEditor.Slice(m_aDemo, m_aDst, m_StartTick, m_EndTick, nullptr, nullptr);
	 
	if(m_Success)
		m_pStorage->RemoveFile(m_aDemo, IStorage::TYPE_SAVE);
}
