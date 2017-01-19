#ifndef CTHREAD_H
#define CTHREAD_H

#include <condition_variable>
#include <mutex>
#include <thread>

#include "IThread.h"

class CCDAudio;

/**
*	CD Audio thread.
*/
class CThread final : public IThread
{
private:
	struct CCommand
	{
		CallbackFn callback;
		int param1;
		int param2;
	};

	enum
	{
		MAX_COMMANDS = 10,
	};

public:
	CThread() = default;
	~CThread() = default;

	bool Init() override;

	void Shutdown() override;

	bool AddThreadItem( CallbackFn callback, int param1, int param2 ) override;

private:
	static void ThreadFunc( CThread* pThread );

	void Run();

private:
	std::thread m_Thread;
	std::mutex m_Mutex;
	std::condition_variable m_Condition;

	volatile bool m_bThreadRunning = false;
	volatile bool m_bThreadStopped = false;

	bool m_bSet = false;

	//These are copied into the thread version to be executed.
	CCommand m_Commands[ MAX_COMMANDS ] = {};
	size_t m_uiNumCommands = 0;

	CCommand m_ThreadCommands[ MAX_COMMANDS ] = {};
	size_t m_uiNumThreadCommands = 0;

private:
	CThread( const CThread& ) = delete;
	CThread& operator=( const CThread& ) = delete;
};

#endif //CTHREAD_H
