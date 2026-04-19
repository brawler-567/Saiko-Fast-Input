 
 
#ifndef ENGINE_SHARED_LINEREADER_H
#define ENGINE_SHARED_LINEREADER_H
#include <base/types.h>

 
class CLineReader
{
	char *m_pBuffer;
	unsigned m_BufferPos;
	bool m_ReadLastLine;

public:
	CLineReader();
	~CLineReader();

	bool OpenFile(IOHANDLE File);
	void OpenBuffer(char *pBuffer);  

	const char *Get();  
};
#endif
