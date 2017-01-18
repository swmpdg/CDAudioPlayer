#include <iostream>
#include <thread>

#include <fmod.hpp>
#include <fmod_errors.h>

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
	std::cout << "CD Audio player" << std::endl << "FMOD Studio, copyright © Firelight Technologies Pty, Ltd., 2012-2016." << std::endl;

	FMOD::System     *system;
	FMOD::Sound      *sound1;
	FMOD::Channel    *channel = 0;
	FMOD_RESULT       result;
	unsigned int      version;
	void             *extradriverdata = 0;

	/*
	Create a System object and initialize
	*/
	result = FMOD::System_Create( &system );
	ERRCHECK( result );

	result = system->getVersion( &version );
	ERRCHECK( result );

	if( version < FMOD_VERSION )
	{
		printf( "FMOD lib version %08x doesn't match header version %08x", version, FMOD_VERSION );
		return 0;
	}

	result = system->init( 32, FMOD_INIT_NORMAL, extradriverdata );
	ERRCHECK( result );

	const char* pszMedia = "../media/Half-Life13.mp3";

	result = system->createSound( pszMedia, FMOD_CREATESTREAM, nullptr, &sound1 );
	ERRCHECK( result );

	result = system->playSound( sound1, 0, false, &channel );
	ERRCHECK( result );

	bool bIsPlaying = false;

	unsigned int uiSeconds;

	result = sound1->getLength( &uiSeconds, FMOD_TIMEUNIT_MS );
	ERRCHECK( result );

	//Milliseconds to Seconds.
	uiSeconds /= 1000;

	const unsigned int uiEndSecond = uiSeconds % 60;
	const unsigned int uiEndMinute = uiSeconds / 60;

	printf( "%2u:%2u/%2u:%2u", 0, 0, uiEndMinute, uiEndSecond );

	/*
	Main loop
	*/
	do
	{
		result = system->update();
		ERRCHECK( result );

		result = channel->isPlaying( &bIsPlaying );

		if( result == FMOD_OK )
		{
			unsigned int uiPos;

			result = channel->getPosition( &uiPos, FMOD_TIMEUNIT_MS );
			ERRCHECK( result );

			uiPos /= 1000;

			const unsigned int uiSecond = uiPos % 60;
			const unsigned int uiMinute = uiPos / 60;

			printf( "\b\b\b\b\b\b\b\b\b\b\b%02u:%02u/%02u:%02u", uiMinute, uiSecond, uiEndMinute, uiEndSecond );

			std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
		}
		//Once the sound has stopped the result code will be invalid handle, if it was reused for another sound it'll be stolen so avoid triggering error handling for those.
		else if( ( result != FMOD_ERR_INVALID_HANDLE ) && ( result != FMOD_ERR_CHANNEL_STOLEN ) )
		{
			ERRCHECK( result );
		}
	}
	while( bIsPlaying );

	/*
	Shut down
	*/
	result = sound1->release();
	ERRCHECK( result );
	result = system->close();
	ERRCHECK( result );
	result = system->release();
	ERRCHECK( result );

	std::cout << std::endl << "Press ENTER to continue..." << std::endl;

	std::cin.get();

	return 0;
}
