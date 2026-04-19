#ifndef ENGINE_SERVER_DATABASES_CONNECTION_H
#define ENGINE_SERVER_DATABASES_CONNECTION_H

#include "connection_pool.h"

#include <engine/shared/protocol.h>

#include <memory>

enum
{
	 
	MAX_NAME_LENGTH_SQL = MAX_NAME_LENGTH - 1,
};

class IConsole;

 
class IDbConnection
{
public:
	IDbConnection(const char *pPrefix);
	virtual ~IDbConnection() = default;
	IDbConnection &operator=(const IDbConnection &) = delete;
	virtual void Print(IConsole *pConsole, const char *pMode) = 0;

	 
	const char *GetPrefix() const { return m_aPrefix; }
	virtual const char *BinaryCollate() const = 0;
	 
	virtual void ToUnixTimestamp(const char *pTimestamp, char *aBuf, unsigned int BufferSize) = 0;
	 
	 
	 
	virtual const char *InsertTimestampAsUtc() const = 0;
	 
	virtual const char *CollateNocase() const = 0;
	 
	virtual const char *InsertIgnore() const = 0;
	 
	virtual const char *Random() const = 0;
	 
	virtual const char *MedianMapTime(char *pBuffer, int BufferSize) const = 0;
	virtual const char *False() const = 0;
	virtual const char *True() const = 0;

	 
	 
	 
	virtual bool Connect(char *pError, int ErrorSize) = 0;
	 
	virtual void Disconnect() = 0;

	 
	 
	 
	virtual bool PrepareStatement(const char *pStmt, char *pError, int ErrorSize) = 0;

	 
	virtual void BindString(int Idx, const char *pString) = 0;
	virtual void BindBlob(int Idx, unsigned char *pBlob, int Size) = 0;
	virtual void BindInt(int Idx, int Value) = 0;
	virtual void BindInt64(int Idx, int64_t Value) = 0;
	virtual void BindFloat(int Idx, float Value) = 0;
	virtual void BindNull(int Idx) = 0;

	 
	virtual void Print() = 0;

	 
	 
	 
	 
	virtual bool Step(bool *pEnd, char *pError, int ErrorSize) = 0;
	 
	 
	 
	virtual bool ExecuteUpdate(int *pNumUpdated, char *pError, int ErrorSize) = 0;

	virtual bool IsNull(int Col) = 0;
	virtual float GetFloat(int Col) = 0;
	virtual int GetInt(int Col) = 0;
	virtual int64_t GetInt64(int Col) = 0;
	 
	virtual void GetString(int Col, char *pBuffer, int BufferSize) = 0;
	 
	virtual int GetBlob(int Col, unsigned char *pBuffer, int BufferSize) = 0;

	 
	virtual bool AddPoints(const char *pPlayer, int Points, char *pError, int ErrorSize) = 0;

private:
	char m_aPrefix[64];

protected:
	void FormatCreateRace(char *aBuf, unsigned int BufferSize, bool Backup) const;
	void FormatCreateTeamrace(char *aBuf, unsigned int BufferSize, const char *pIdType, bool Backup) const;
	void FormatCreateMaps(char *aBuf, unsigned int BufferSize) const;
	void FormatCreateSaves(char *aBuf, unsigned int BufferSize, bool Backup) const;
	void FormatCreatePoints(char *aBuf, unsigned int BufferSize) const;
};

bool MysqlAvailable();
int MysqlInit();
void MysqlUninit();

std::unique_ptr<IDbConnection> CreateSqliteConnection(const char *pFilename, bool Setup);
 
std::unique_ptr<IDbConnection> CreateMysqlConnection(CMysqlConfig Config);

#endif  
