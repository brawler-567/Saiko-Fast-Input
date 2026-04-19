 
 
#include "memheap.h"

#include <base/math.h>
#include <base/system.h>

#include <cstdint>
#include <cstdlib>

 
void CHeap::NewChunk(size_t ChunkSize)
{
	 
	 
	CChunk *pChunk = static_cast<CChunk *>(malloc(sizeof(CChunk) + ChunkSize));
	if(!pChunk)
		return;
	pChunk->m_pMemory = static_cast<char *>(static_cast<void *>(pChunk + 1));
	pChunk->m_pCurrent = pChunk->m_pMemory;
	pChunk->m_pEnd = pChunk->m_pMemory + ChunkSize;
	pChunk->m_pNext = nullptr;

	pChunk->m_pNext = m_pCurrent;
	m_pCurrent = pChunk;
}

 
void *CHeap::AllocateFromChunk(unsigned int Size, unsigned Alignment)
{
	size_t Offset = reinterpret_cast<uintptr_t>(m_pCurrent->m_pCurrent) % Alignment;
	if(Offset)
		Offset = Alignment - Offset;

	 
	if(m_pCurrent->m_pCurrent + Offset + Size > m_pCurrent->m_pEnd)
		return nullptr;

	 
	char *pMem = m_pCurrent->m_pCurrent + Offset;
	m_pCurrent->m_pCurrent += Offset + Size;
	return pMem;
}

 
CHeap::CHeap()
{
	m_pCurrent = nullptr;
	Reset();
}

CHeap::~CHeap()
{
	Clear();
}

void CHeap::Reset()
{
	Clear();
	NewChunk(CHUNK_SIZE);
}

 
void CHeap::Clear()
{
	while(m_pCurrent)
	{
		CChunk *pNext = m_pCurrent->m_pNext;
		free(m_pCurrent);
		m_pCurrent = pNext;
	}
}

 
void *CHeap::Allocate(unsigned Size, unsigned Alignment)
{
	 
	void *pMem = AllocateFromChunk(Size, Alignment);
	if(!pMem)
	{
		 
		NewChunk(maximum<size_t>(CHUNK_SIZE, Size + Alignment));

		 
		pMem = AllocateFromChunk(Size, Alignment);
	}

	return pMem;
}

const char *CHeap::StoreString(const char *pSrc)
{
	const int Size = str_length(pSrc) + 1;
	char *pMem = static_cast<char *>(Allocate(Size));
	str_copy(pMem, pSrc, Size);
	return pMem;
}
