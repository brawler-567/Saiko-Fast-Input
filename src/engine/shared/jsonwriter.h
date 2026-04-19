 
 
#ifndef ENGINE_SHARED_JSONWRITER_H
#define ENGINE_SHARED_JSONWRITER_H

#include <base/types.h>

#include <stack>
#include <string>

 
class CJsonWriter
{
	enum EJsonStateKind
	{
		STATE_OBJECT,
		STATE_ARRAY,
		STATE_ATTRIBUTE,
	};

	struct SState
	{
		EJsonStateKind m_Kind;
		bool m_Empty = true;

		SState(EJsonStateKind Kind) :
			m_Kind(Kind)
		{
		}
	};

	std::stack<SState> m_States;
	int m_Indentation;

	bool CanWriteDatatype();
	void WriteInternalEscaped(const char *pStr);
	void WriteIndent(bool EndElement);
	void PushState(EJsonStateKind NewState);
	SState *TopState();
	EJsonStateKind PopState();
	void CompleteDataType();

protected:
	 
	virtual void WriteInternal(const char *pStr, int Length = -1) = 0;

public:
	CJsonWriter();
	virtual ~CJsonWriter() = default;

	 
	 

	 
	void BeginObject();
	 
	void EndObject();

	 
	void BeginArray();
	 
	void EndArray();

	 
	 
	 
	 
	void WriteAttribute(const char *pName);

	 
	 
	 
	 
	void WriteStrValue(const char *pValue);
	void WriteIntValue(int Value);
	void WriteBoolValue(bool Value);
	void WriteNullValue();
};

 
class CJsonFileWriter : public CJsonWriter
{
	IOHANDLE m_IO;

protected:
	void WriteInternal(const char *pStr, int Length = -1) override;

public:
	 
	CJsonFileWriter(IOHANDLE IO);
	~CJsonFileWriter() override;
};

 
class CJsonStringWriter : public CJsonWriter
{
	std::string m_OutputString;
	bool m_RetrievedOutput = false;

protected:
	void WriteInternal(const char *pStr, int Length = -1) override;

public:
	CJsonStringWriter() = default;
	~CJsonStringWriter() override = default;
	std::string &&GetOutputString();
};

#endif
