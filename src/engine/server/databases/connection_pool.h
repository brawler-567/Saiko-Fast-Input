#ifndef ENGINE_SERVER_DATABASES_CONNECTION_POOL_H
#define ENGINE_SERVER_DATABASES_CONNECTION_POOL_H

#include <base/tl/threading.h>

#include <atomic>
#include <memory>
#include <vector>

class IDbConnection;

struct ISqlResult
{
	 
	 
	 
	std::atomic_bool m_Completed{false};
	 
	bool m_Success = false;

	virtual ~ISqlResult() = default;
};

struct ISqlData
{
	ISqlData(std::shared_ptr<ISqlResult> pResult) :
		m_pResult(std::move(pResult))
	{
	}
	virtual ~ISqlData() = default;

	mutable std::shared_ptr<ISqlResult> m_pResult;
};

enum Write
{
	 
	BACKUP_FIRST,
	 
	NORMAL,
	 
	NORMAL_SUCCEEDED,
	 
	NORMAL_FAILED,
};

class IConsole;

struct CMysqlConfig
{
	char m_aDatabase[64];
	char m_aPrefix[64];
	char m_aUser[64];
	char m_aPass[64];
	char m_aIp[64];
	char m_aBindaddr[128];
	int m_Port;
	bool m_Setup;
};

class CDbConnectionPool
{
public:
	CDbConnectionPool();
	~CDbConnectionPool();
	CDbConnectionPool &operator=(const CDbConnectionPool &) = delete;

	 
	typedef bool (*FRead)(IDbConnection *, const ISqlData *, char *pError, int ErrorSize);
	typedef bool (*FWrite)(IDbConnection *, const ISqlData *, Write, char *pError, int ErrorSize);

	enum Mode
	{
		READ,
		WRITE,
		WRITE_BACKUP,
		NUM_MODES,
	};

	void Print(IConsole *pConsole, Mode DatabaseMode);

	void RegisterSqliteDatabase(Mode DatabaseMode, const char aFilename[64]);
	void RegisterMysqlDatabase(Mode DatabaseMode, const CMysqlConfig *pMysqlConfig);

	void Execute(
		FRead pFunc,
		std::unique_ptr<const ISqlData> pSqlRequestData,
		const char *pName);
	 
	 
	void ExecuteWrite(
		FWrite pFunc,
		std::unique_ptr<const ISqlData> pSqlRequestData,
		const char *pName);

	void OnShutdown();

	friend class CWorker;
	friend class CBackup;

private:
	static bool ExecSqlFunc(IDbConnection *pConnection, struct CSqlExecData *pData, Write w);

	 
	 
	int m_InsertIdx = 0;

	bool m_Shutdown = false;

	struct CSharedData
	{
		 
		 
		 
		 
		 
		std::atomic_bool m_Shutdown{false};
		 
		 
		CSemaphore m_NumBackup;
		 
		 
		CSemaphore m_NumWorker;

		 
		std::unique_ptr<struct CSqlExecData> m_aQueries[512];
	};

	std::shared_ptr<CSharedData> m_pShared;
	void *m_pWorkerThread = nullptr;
	void *m_pBackupThread = nullptr;
};

#endif  
