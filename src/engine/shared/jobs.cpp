 
 
#include "jobs.h"

#include <algorithm>

IJob::IJob() :
	m_pNext(nullptr),
	m_State(STATE_QUEUED),
	m_Abortable(false)
{
}

IJob::~IJob() = default;

IJob::EJobState IJob::State() const
{
	return m_State;
}

bool IJob::Done() const
{
	EJobState State = m_State;
	return State != STATE_QUEUED && State != STATE_RUNNING;
}

bool IJob::Abort()
{
	if(!IsAbortable())
		return false;

	m_State = STATE_ABORTED;
	return true;
}

void IJob::Abortable(bool Abortable)
{
	m_Abortable = Abortable;
}

bool IJob::IsAbortable() const
{
	return m_Abortable;
}

CJobPool::CJobPool()
{
	m_Shutdown = true;
}

CJobPool::~CJobPool()
{
	if(!m_Shutdown)
	{
		Shutdown();
	}
}

void CJobPool::WorkerThread(void *pUser)
{
	static_cast<CJobPool *>(pUser)->RunLoop();
}

void CJobPool::RunLoop()
{
	while(true)
	{
		 
		sphore_wait(&m_Semaphore);

		 
		std::shared_ptr<IJob> pJob = nullptr;
		{
			const CLockScope LockScope(m_Lock);
			if(m_pFirstJob)
			{
				pJob = m_pFirstJob;
				m_pFirstJob = m_pFirstJob->m_pNext;
				 
				pJob->m_pNext = nullptr;
				if(!m_pFirstJob)
					m_pLastJob = nullptr;
			}
		}

		if(pJob)
		{
			IJob::EJobState OldStateQueued = IJob::STATE_QUEUED;
			if(!pJob->m_State.compare_exchange_strong(OldStateQueued, IJob::STATE_RUNNING))
			{
				if(OldStateQueued == IJob::STATE_ABORTED)
				{
					 
					pJob->m_State = IJob::STATE_ABORTED;
					continue;
				}
				dbg_assert_failed("Job state invalid. Job was reused or uninitialized.");
			}

			 
			{
				const CLockScope LockScope(m_LockRunning);
				m_RunningJobs.push_back(pJob);
			}
			pJob->Run();
			{
				const CLockScope LockScope(m_LockRunning);
				m_RunningJobs.erase(std::find(m_RunningJobs.begin(), m_RunningJobs.end(), pJob));
			}

			 
			IJob::EJobState OldStateRunning = IJob::STATE_RUNNING;
			if(!pJob->m_State.compare_exchange_strong(OldStateRunning, IJob::STATE_DONE))
			{
				if(OldStateRunning != IJob::STATE_ABORTED)
				{
					dbg_assert_failed("Job state invalid, must be either running or aborted");
				}
			}
		}
		else if(m_Shutdown)
		{
			 
			break;
		}
	}
}

void CJobPool::Init(int NumThreads)
{
	dbg_assert(m_Shutdown, "Job pool already running");
	m_Shutdown = false;

	const CLockScope LockScope(m_Lock);
	sphore_init(&m_Semaphore);
	m_pFirstJob = nullptr;
	m_pLastJob = nullptr;

	 
	char aName[16];  
	m_vpThreads.reserve(NumThreads);
	for(int i = 0; i < NumThreads; i++)
	{
		str_format(aName, sizeof(aName), "CJobPool W%d", i);
		m_vpThreads.push_back(thread_init(WorkerThread, this, aName));
	}
}

void CJobPool::Shutdown()
{
	dbg_assert(!m_Shutdown, "Job pool already shut down");
	m_Shutdown = true;

	 
	{
		const CLockScope LockScope(m_Lock);
		std::shared_ptr<IJob> pJob = m_pFirstJob;
		std::shared_ptr<IJob> pPrev = nullptr;
		while(pJob != nullptr)
		{
			std::shared_ptr<IJob> pNext = pJob->m_pNext;
			if(pJob->Abort())
			{
				 
				pJob->m_pNext = nullptr;
				if(pPrev)
				{
					pPrev->m_pNext = pNext;
				}
				else
				{
					m_pFirstJob = pNext;
				}
			}
			else
			{
				pPrev = pJob;
			}
			pJob = pNext;
		}
		m_pLastJob = pPrev;
	}

	 
	{
		const CLockScope LockScope(m_LockRunning);
		for(const std::shared_ptr<IJob> &pJob : m_RunningJobs)
		{
			pJob->Abort();
		}
	}

	 
	for(size_t i = 0; i < m_vpThreads.size(); i++)
	{
		sphore_signal(&m_Semaphore);
	}

	 
	for(void *pThread : m_vpThreads)
	{
		thread_wait(pThread);
	}

	m_vpThreads.clear();
	sphore_destroy(&m_Semaphore);
}

void CJobPool::Add(std::shared_ptr<IJob> pJob)
{
	if(m_Shutdown)
	{
		 
		pJob->Abort();
		return;
	}

	 
	{
		const CLockScope LockScope(m_Lock);
		if(m_pLastJob)
			m_pLastJob->m_pNext = pJob;
		m_pLastJob = std::move(pJob);
		if(!m_pFirstJob)
			m_pFirstJob = m_pLastJob;
	}

	 
	sphore_signal(&m_Semaphore);
}
