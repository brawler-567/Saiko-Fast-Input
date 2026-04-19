 
 
#include "ringbuffer.h"

CRingBufferBase::CItem *CRingBufferBase::NextBlock(CItem *pItem)
{
	if(pItem->m_pNext)
		return pItem->m_pNext;
	return m_pFirst;
}

CRingBufferBase::CItem *CRingBufferBase::PrevBlock(CItem *pItem)
{
	if(pItem->m_pPrev)
		return pItem->m_pPrev;
	return m_pLast;
}

CRingBufferBase::CItem *CRingBufferBase::MergeBack(CItem *pItem)
{
	 
	if(!pItem->m_Free || !pItem->m_pPrev || !pItem->m_pPrev->m_Free)
		return pItem;

	 
	pItem->m_pPrev->m_Size += pItem->m_Size;
	pItem->m_pPrev->m_pNext = pItem->m_pNext;

	 
	if(pItem->m_pNext)
		pItem->m_pNext->m_pPrev = pItem->m_pPrev;
	else
		m_pLast = pItem->m_pPrev;

	if(pItem == m_pProduce)
		m_pProduce = pItem->m_pPrev;

	if(pItem == m_pConsume)
		m_pConsume = pItem->m_pPrev;

	 
	return pItem->m_pPrev;
}

void CRingBufferBase::Init(void *pMemory, int Size, int Flags)
{
	m_Size = Size / sizeof(CItem) * sizeof(CItem);
	m_pBuffer = static_cast<CItem *>(pMemory);
	m_Flags = Flags;
	Clear();
}

void CRingBufferBase::Clear()
{
	m_pFirst = m_pBuffer;
	m_pFirst->m_pPrev = nullptr;
	m_pFirst->m_pNext = nullptr;
	m_pFirst->m_Free = 1;
	m_pFirst->m_Size = m_Size;
	m_pLast = m_pFirst;
	m_pProduce = m_pFirst;
	m_pConsume = m_pFirst;
}

void *CRingBufferBase::Allocate(int Size)
{
	int WantedSize = (Size + sizeof(CItem) + sizeof(CItem) - 1) / sizeof(CItem) * sizeof(CItem);
	CItem *pBlock = nullptr;

	 
	if(WantedSize > m_Size)
		return nullptr;

	while(true)
	{
		 
		if(m_pProduce->m_Free)
		{
			if(m_pProduce->m_Size >= WantedSize)
				pBlock = m_pProduce;
			else
			{
				 
				if(m_pFirst->m_Free && m_pFirst->m_Size >= WantedSize)
					pBlock = m_pFirst;
			}
		}

		if(pBlock)
			break;
		else
		{
			 
			if(m_Flags & FLAG_RECYCLE)
			{
				if(!PopFirst())
					return nullptr;
			}
			else
				return nullptr;
		}
	}

	 

	 
	if(pBlock->m_Size > WantedSize + (int)sizeof(CItem))
	{
		CItem *pNewItem = (CItem *)((char *)pBlock + WantedSize);
		pNewItem->m_pPrev = pBlock;
		pNewItem->m_pNext = pBlock->m_pNext;
		if(pNewItem->m_pNext)
			pNewItem->m_pNext->m_pPrev = pNewItem;
		pBlock->m_pNext = pNewItem;

		pNewItem->m_Free = 1;
		pNewItem->m_Size = pBlock->m_Size - WantedSize;
		pBlock->m_Size = WantedSize;

		if(!pNewItem->m_pNext)
			m_pLast = pNewItem;
	}

	 
	m_pProduce = NextBlock(pBlock);

	 
	pBlock->m_Free = 0;
	return (void *)(pBlock + 1);
}

void CRingBufferBase::SetPopCallback(std::function<void(void *pCurrent)> PopCallback)
{
	m_PopCallback = std::move(PopCallback);
}

int CRingBufferBase::PopFirst()
{
	if(m_pConsume->m_Free)
		return 0;

	if(m_PopCallback)
	{
		m_PopCallback(m_pConsume + 1);
	}

	 
	m_pConsume->m_Free = 1;

	 
	m_pConsume = MergeBack(m_pConsume);

	 
	m_pConsume = NextBlock(m_pConsume);
	while(m_pConsume->m_Free && m_pConsume != m_pProduce)
	{
		m_pConsume = MergeBack(m_pConsume);
		m_pConsume = NextBlock(m_pConsume);
	}

	 
	 
	MergeBack(m_pConsume);
	return 1;
}

void *CRingBufferBase::Prev(void *pCurrent)
{
	CItem *pItem = ((CItem *)pCurrent) - 1;

	while(true)
	{
		pItem = PrevBlock(pItem);
		if(pItem == m_pProduce)
			return nullptr;
		if(!pItem->m_Free)
			return pItem + 1;
	}
}

void *CRingBufferBase::Next(void *pCurrent)
{
	CItem *pItem = ((CItem *)pCurrent) - 1;

	while(true)
	{
		pItem = NextBlock(pItem);
		if(pItem == m_pProduce)
			return nullptr;
		if(!pItem->m_Free)
			return pItem + 1;
	}
}

void *CRingBufferBase::First()
{
	if(m_pConsume->m_Free)
		return nullptr;
	return (void *)(m_pConsume + 1);
}

void *CRingBufferBase::Last()
{
	return Prev(m_pProduce + 1);
}
