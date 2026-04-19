 
 
#include "compression.h"

#include <base/system.h>

#include <iterator>  

 
unsigned char *CVariableInt::Pack(unsigned char *pDst, int i, int DstSize)
{
	if(DstSize <= 0)
		return nullptr;

	DstSize--;
	*pDst = 0;
	if(i < 0)
	{
		*pDst |= 0x40;  
		i = ~i;
	}

	*pDst |= i & 0x3F;  
	i >>= 6;  
	while(i)
	{
		if(DstSize <= 0)
			return nullptr;
		*pDst |= 0x80;  
		DstSize--;
		pDst++;
		*pDst = i & 0x7F;  
		i >>= 7;  
	}

	pDst++;
	return pDst;
}

const unsigned char *CVariableInt::Unpack(const unsigned char *pSrc, int *pInOut, int SrcSize)
{
	if(SrcSize <= 0)
		return nullptr;

	const int Sign = (*pSrc >> 6) & 1;
	*pInOut = *pSrc & 0x3F;
	SrcSize--;

	static const int s_aMasks[] = {0x7F, 0x7F, 0x7F, 0x0F};
	static const int s_aShifts[] = {6, 6 + 7, 6 + 7 + 7, 6 + 7 + 7 + 7};

	for(unsigned i = 0; i < std::size(s_aMasks); i++)
	{
		if(!(*pSrc & 0x80))
			break;
		if(SrcSize <= 0)
			return nullptr;
		SrcSize--;
		pSrc++;
		*pInOut |= (*pSrc & s_aMasks[i]) << s_aShifts[i];
	}

	pSrc++;
	*pInOut ^= -Sign;  
	return pSrc;
}

long CVariableInt::Decompress(const void *pSrc, int SrcSize, void *pDst, int DstSize)
{
	dbg_assert(DstSize % sizeof(int) == 0, "invalid bounds");

	const unsigned char *pCharSrc = (unsigned char *)pSrc;
	const unsigned char *pCharSrcEnd = pCharSrc + SrcSize;
	int *pIntDst = (int *)pDst;
	const int *pIntDstEnd = pIntDst + DstSize / sizeof(int);  
	while(pCharSrc < pCharSrcEnd)
	{
		if(pIntDst >= pIntDstEnd)
			return -1;
		pCharSrc = CVariableInt::Unpack(pCharSrc, pIntDst, pCharSrcEnd - pCharSrc);
		if(!pCharSrc)
			return -1;
		pIntDst++;
	}
	return (long)((unsigned char *)pIntDst - (unsigned char *)pDst);
}

long CVariableInt::Compress(const void *pSrc, int SrcSize, void *pDst, int DstSize)
{
	dbg_assert(SrcSize % sizeof(int) == 0, "invalid bounds");

	const int *pIntSrc = (int *)pSrc;
	unsigned char *pCharDst = (unsigned char *)pDst;
	const unsigned char *pCharDstEnd = pCharDst + DstSize;
	SrcSize /= sizeof(int);
	while(SrcSize)
	{
		pCharDst = CVariableInt::Pack(pCharDst, *pIntSrc, pCharDstEnd - pCharDst);
		if(!pCharDst)
			return -1;
		SrcSize--;
		pIntSrc++;
	}
	return (long)(pCharDst - (unsigned char *)pDst);
}
