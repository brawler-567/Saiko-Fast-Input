#ifndef BASE_LOGGER_H
#define BASE_LOGGER_H

#include "lock.h"
#include "log.h"

#include <atomic>
#include <memory>
#include <string>
#include <vector>

typedef void *IOHANDLE;

 
class CLogMessage
{
public:
	 
	LEVEL m_Level;
	bool m_HaveColor;
	 
	LOG_COLOR m_Color;
	char m_aTimestamp[80];
	char m_aSystem[32];
	 
	char m_aLine[4096];
	int m_TimestampLength;
	int m_SystemLength;
	 
	int m_LineLength;
	int m_LineMessageOffset;

	 
	const char *Message() const
	{
		return m_aLine + m_LineMessageOffset;
	}
};

class CLogFilter
{
public:
	 
	std::atomic_int m_MaxLevel{LEVEL_INFO};

	bool Filters(const CLogMessage *pMessage);
};

class ILogger
{
protected:
	CLogFilter m_Filter;

public:
	virtual ~ILogger() = default;

	 
	void SetFilter(const CLogFilter &Filter)
	{
		m_Filter.m_MaxLevel.store(Filter.m_MaxLevel.load(std::memory_order_relaxed), std::memory_order_relaxed);
		OnFilterChange();
	}

	 
	virtual void Log(const CLogMessage *pMessage) = 0;
	 
	virtual void GlobalFinish() {}
	 
	virtual void OnFilterChange() {}
};

 
void log_set_global_logger(ILogger *logger);

 
void log_set_global_logger_default();

 
void log_global_logger_finish();

 
ILogger *log_get_scope_logger();

 
void log_set_scope_logger(ILogger *logger);

 
std::unique_ptr<ILogger> log_logger_android();

 
std::unique_ptr<ILogger> log_logger_collection(std::vector<std::shared_ptr<ILogger>> &&vpLoggers);

 
std::unique_ptr<ILogger> log_logger_file(IOHANDLE file);

 
std::unique_ptr<ILogger> log_logger_stdout();

 
std::unique_ptr<ILogger> log_logger_windows_debugger();

 
std::unique_ptr<ILogger> log_logger_noop();

 
class CFutureLogger : public ILogger
{
private:
	std::shared_ptr<ILogger> m_pLogger;
	std::vector<CLogMessage> m_vPending;
	CLock m_PendingLock;

public:
	 
	void Set(std::shared_ptr<ILogger> pLogger) REQUIRES(!m_PendingLock);
	void Log(const CLogMessage *pMessage) override REQUIRES(!m_PendingLock);
	void GlobalFinish() override;
	void OnFilterChange() override;
};

 
class CMemoryLogger : public ILogger
{
	ILogger *m_pParentLogger = nullptr;
	std::vector<CLogMessage> m_vMessages GUARDED_BY(m_MessagesMutex);
	CLock m_MessagesMutex;

public:
	void SetParent(ILogger *pParentLogger) { m_pParentLogger = pParentLogger; }
	void Log(const CLogMessage *pMessage) override REQUIRES(!m_MessagesMutex);
	std::vector<CLogMessage> Lines() REQUIRES(!m_MessagesMutex);
	std::string ConcatenatedLines() REQUIRES(!m_MessagesMutex);
};

 
class CLogScope
{
	ILogger *old_scope_logger;
	ILogger *new_scope_logger;

public:
	CLogScope(ILogger *logger) :
		old_scope_logger(log_get_scope_logger()),
		new_scope_logger(logger)
	{
		log_set_scope_logger(new_scope_logger);
	}
	~CLogScope()
	{
		log_set_scope_logger(old_scope_logger);
	}
	CLogScope(const CLogScope &) = delete;
};
#endif  
