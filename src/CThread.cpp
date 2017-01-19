#include <iostream>
#include <string>

#include "Sound.h"

#include "CThread.h"

bool CThread::Init()
{
	m_bThreadRunning = true;

	cdaudio->Init();

	try
	{
		m_Thread = std::move( std::thread( &CThread::ThreadFunc, this ) );

		m_Thread.detach();

		return true;
	}
	catch( const std::system_error& e )
	{
		std::cout << "Failed to start Audio thread: " << e.code().message() << std::endl;

		return false;
	}
}

void CThread::Shutdown()
{
	m_bThreadStopped = false;
	m_bThreadRunning = false;

	m_Mutex.lock();
	m_bSet = true;
	m_Condition.notify_all();
	m_Mutex.unlock();

	for( int i = 99; i > 0; --i )
	{
		std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

		if( m_bThreadStopped )
			break;
	}

	cdaudio->Shutdown();
}

bool CThread::AddThreadItem( CallbackFn callback, int param1, int param2 )
{
	std::lock_guard<std::mutex> lock( m_Mutex );

	for( size_t uiIndex = 0; uiIndex < m_uiNumCommands; ++uiIndex )
	{
		//Already in the list, don't add.
		if( m_Commands[ uiIndex ].callback == callback )
			return false;
	}

	if( m_uiNumCommands >= MAX_COMMANDS )
	{
		return false;
	}

	auto& command = m_Commands[ m_uiNumCommands++ ];

	command.callback = callback;
	command.param1 = param1;
	command.param2 = param2;

	m_bSet = true;

	m_Condition.notify_all();

	return true;
}

void CThread::ThreadFunc( CThread* pThread )
{
	pThread->Run();
}

void CThread::Run()
{
	while( m_bThreadRunning )
	{
		std::unique_lock<std::mutex> lock( m_Mutex );

		if( !m_bSet )
		{
			m_Condition.wait( lock,
							  [ & ]()
			{
				return !m_bSet;
			}
			);
		}

		memcpy( m_ThreadCommands, m_Commands, sizeof( m_ThreadCommands ) );
		m_uiNumThreadCommands = m_uiNumCommands;

		m_uiNumCommands = 0;

		lock.unlock();

		for( size_t uiCommand = 0; uiCommand < m_uiNumThreadCommands; ++uiCommand )
		{
			auto& command = m_ThreadCommands[ uiCommand ];

			( GetInteralCDAudio()->*command.callback )( command.param1, command.param2 );
		}
	}

	m_bThreadStopped = true;
}
