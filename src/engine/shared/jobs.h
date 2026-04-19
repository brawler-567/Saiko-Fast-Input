 
 
#ifndef ENGINE_SHARED_JOBS_H
#define ENGINE_SHARED_JOBS_H

#include <base/lock.h>
#include <base/system.h>

#include <atomic>
#include <deque>
#include <memory>
#include <vector>

 
class IJob
{
	friend class CJobPool;

public:
	 
	enum EJobState
	{
		 
		STATE_QUEUED = 0,

		 
		STATE_RUNNING,

		 
		STATE_DONE,

		 
		STATE_ABORTED,
	};

private:
	std::shared_ptr<IJob> m_pNext;
	std::atomic<EJobState> m_State;
	std::atomic<bool> m_Abortable;

protected:
	 
	virtual void Run() = 0;

	 
	void Abortable(bool Abortable);

public:
	IJob();
	virtual ~IJob();

	IJob(const IJob &Other) = delete;
	IJob &operator=(const IJob &Other) = delete;

	 
	EJobState State() const;

	 
	bool Done() const;

	 
	virtual bool Abort();

	 
	bool IsAbortable() const;
};

 
class CJobPool
{
	std::vector<void *> m_vpThreads;
	std::atomic<bool> m_Shutdown;

	CLock m_Lock;
	SEMAPHORE m_Semaphore;
	std::shared_ptr<IJob> m_pFirstJob GUARDED_BY(m_Lock);
	std::shared_ptr<IJob> m_pLastJob GUARDED_BY(m_Lock);

	CLock m_LockRunning;
	std::deque<std::shared_ptr<IJob>> m_RunningJobs GUARDED_BY(m_LockRunning);

	static void WorkerThread(void *pUser) NO_THREAD_SAFETY_ANALYSIS;
	void RunLoop() NO_THREAD_SAFETY_ANALYSIS;

public:
	CJobPool();
	~CJobPool();

	 
	void Init(int NumThreads) REQUIRES(!m_Lock);

	 
	void Shutdown() REQUIRES(!m_Lock) REQUIRES(!m_LockRunning);

	 
	void Add(std::shared_ptr<IJob> pJob) REQUIRES(!m_Lock);
};
#endif
