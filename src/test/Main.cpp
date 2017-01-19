#include <chrono>
#include <iostream>
#include <thread>

#include <fmod.hpp>
#include <fmod_errors.h>

#include "CCDAudio.h"
#include "Sound.h"
#include "Stubs.h"

void ERRCHECK_fn( FMOD_RESULT result, const char *file, int line )
{
	if( result != FMOD_OK )
	{
		printf( "%s(%d): FMOD error %d - %s", file, line, result, FMOD_ErrorString( result ) );

		std::cin.get();

		exit( 1 );
	}
}

#define ERRCHECK(_result) ERRCHECK_fn(_result, __FILE__, __LINE__)

int main( int iArgc, char* pszArgV[] )
{
	//Simple test for the CD Audio system.

	std::cout << "CD Audio player" << std::endl << "FMOD Studio, copyright © Firelight Technologies Pty, Ltd., 2012-2016." << std::endl;

	//Initialize Sound system. Required so cvars manage memory properly.
	S_Init();
	//Initialize CD Audio system.
	CDAudio_Init();

	//Play a sound.
	cdaudio->PlayFile( "media\\Suspense07", true );

	auto start = std::chrono::system_clock::now();

	//Play for 30 seconds.
	auto end = start + std::chrono::duration_cast<decltype( start )::duration>( std::chrono::seconds( 30 ) );

	realtime = std::chrono::duration_cast<std::chrono::milliseconds>( start.time_since_epoch() ).count() / 1000.0;

	unsigned int uiPlayedCount = 0;

	for( auto now = start; now < end; now = std::chrono::system_clock::now() )
	{
		//Update real time so time based operations work properly.
		realtime = std::chrono::duration_cast<std::chrono::milliseconds>( now.time_since_epoch() ).count() / 1000.0;
		cdaudio->Frame();

		if( ( now - start ) >= std::chrono::seconds( 10 ) && uiPlayedCount == 0 )
		{
			++uiPlayedCount;

			//Start another sound now that the other one has finished.
			cdaudio->PlayFile( "media\\Suspense03", false );
		}
		else if( ( now - start ) >= std::chrono::seconds( 20 ) && uiPlayedCount == 1 )
		{
			++uiPlayedCount;

			//Start another sound now that the other one has finished.
			cdaudio->PlayFile( "media\\Half-Life13", false );
		}
	}

	//Shutdown.
	CDAudio_Shutdown();

	std::cout << std::endl << "Press ENTER to continue..." << std::endl;

	std::cin.get();

	return 0;
}
