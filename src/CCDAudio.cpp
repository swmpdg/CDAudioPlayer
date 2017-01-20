#include <cstring>
#include <iostream>
#include <string>

#include <fmod_errors.h>

#include "Stubs.h"
#include "Sound.h"
#include "CThread.h"

#include "CCDAudio.h"

static CThread g_Thread;

static IThread* thread = &g_Thread;

static CCDAudio g_CDAudio;

ICDAudio* cdaudio = &g_CDAudio;

void CDAudio_Init( void )
{
	thread->Init();
}

void CDAudio_Shutdown( void )
{
	thread->Shutdown();
}

void CDAudio_Play( int track, int looping )
{
	cdaudio->Play( track, looping != 0 );
}

void CDAudio_Pause( void )
{
	cdaudio->Pause();
}

void CDAudio_Resume( void )
{
	cdaudio->Resume();
}

void CD_Command_f( void )
{
	GetInteralCDAudio()->CD_f();
}

CCDAudio* GetInteralCDAudio()
{
	return &g_CDAudio;
}

cvar_t MP3Volume{ "MP3Volume", "0.8" };
cvar_t MP3FadeTime{ "MP3FadeTime", "2.0" };
cvar_t bgmvolume{ "bgmvolume", "1.0" };

void S_Init( void )
{
	Cvar_Register( &MP3Volume );
	Cvar_Register( &MP3FadeTime );
	Cvar_Register( &bgmvolume );
}

#define MAX_MP3 200

const char* g_pszMP3trackFileMap[ MAX_MP3 ];

int g_iMP3FirstMalloc = 0;
int g_iMP3NumTracks = 0;

int MP3_GetTrack( const char *pszTrack )
{
	char szTemp[ 4096 ];

	const char* pszFormat = "%s";

	if( !strstr( pszTrack, ".mp3" ) )
		pszFormat = "%s.mp3";

	snprintf( szTemp, sizeof( szTemp ), pszFormat, pszTrack );
	szTemp[ sizeof( szTemp ) - 1 ] = '\0';
	COM_FixSlashes( szTemp );

	for( int i = 0; i < MAX_MP3; ++i )
	{
		if( !g_pszMP3trackFileMap[ i ] )
		{
			g_pszMP3trackFileMap[ i ] = Mem_Strdup( szTemp );
			g_iMP3NumTracks = i + 1;
			return i;
		}

		if( !strcmp( g_pszMP3trackFileMap[ i ], szTemp ) )
			return i;
	}

	return 0;
}

bool CCDAudio::Init()
{
	m_MP3.tracknum = 0;
	m_MP3.inuse = false;
	m_MP3.suspended = false;
	m_MP3.playing = false;
	m_MP3.trackname[ 0 ] = '\0';
	m_MP3.looping = false;
	m_MP3.volume = 100.0f;

	memset( g_pszMP3trackFileMap, 0, sizeof( g_pszMP3trackFileMap ) );

	g_pszMP3trackFileMap[ 0 ] = "";
	g_pszMP3trackFileMap[ 1 ] = "";
	g_pszMP3trackFileMap[ 2 ] = "media\\Half-Life01.mp3";
	g_pszMP3trackFileMap[ 3 ] = "media\\Prospero01.mp3";
	g_pszMP3trackFileMap[ 4 ] = "media\\Half-Life12.mp3";
	g_pszMP3trackFileMap[ 5 ] = "media\\Half-Life07.mp3";
	g_pszMP3trackFileMap[ 8 ] = "media\\Suspense03.mp3";
	g_pszMP3trackFileMap[ 9 ] = "media\\Half-Life09.mp3";
	g_pszMP3trackFileMap[ 10 ] = "media\\Half-Life02.mp3";
	g_pszMP3trackFileMap[ 11 ] = "media\\Half-Life13.mp3";
	g_pszMP3trackFileMap[ 12 ] = "media\\Half-Life04.mp3";
	g_pszMP3trackFileMap[ 13 ] = "media\\Half-Life15.mp3";
	g_pszMP3trackFileMap[ 16 ] = "media\\Suspense02.mp3";
	g_pszMP3trackFileMap[ 17 ] = "media\\Half-Life03.mp3";
	g_pszMP3trackFileMap[ 18 ] = "media\\Half-Life08.mp3";
	g_pszMP3trackFileMap[ 19 ] = "media\\Prospero02.mp3";
	g_pszMP3trackFileMap[ 20 ] = "media\\Half-Life05.mp3";
	g_pszMP3trackFileMap[ 21 ] = "media\\Prospero04.mp3";
	g_pszMP3trackFileMap[ 24 ] = "media\\Prospero03.mp3";
	g_pszMP3trackFileMap[ 25 ] = "media\\Half-Life17.mp3";
	g_pszMP3trackFileMap[ 6 ] = "media\\Half-Life10.mp3";
	g_pszMP3trackFileMap[ 26 ] = "media\\Prospero05.mp3";
	g_pszMP3trackFileMap[ 7 ] = "media\\Suspense01.mp3";
	g_pszMP3trackFileMap[ 14 ] = "media\\Half-Life14.mp3";
	g_pszMP3trackFileMap[ 27 ] = "media\\Suspense05.mp3";
	g_pszMP3trackFileMap[ 15 ] = "media\\Half-Life16.mp3";
	g_pszMP3trackFileMap[ 22 ] = "media\\Half-Life11.mp3";
	g_pszMP3trackFileMap[ 28 ] = "media\\Suspense07.mp3";
	g_pszMP3trackFileMap[ 23 ] = "media\\Half-Life06.mp3";

	m_flPlayTime = 0;

	g_iMP3FirstMalloc = 29;
	g_iMP3NumTracks = 29;

	m_dStartTime = 0;
	m_dPauseTime = 0;

	if( MP3_Init() )
	{
		MP3_SetVolume( MP3Volume.value );
	}

	m_bInitialized = true;
	m_bEnabled = true;

	thread->AddThreadItem( &CCDAudio::_Init, 0, 0 );

	return true;
}

void CCDAudio::Shutdown()
{
	if( m_MP3.inuse )
	{
		MP3_SetPause( true );
	}

	MP3_Shutdown();

	if( m_bInitialized )
	{
		ResetCDTimes();
	}
}

void CCDAudio::Play( int trackNum, bool looping )
{
	m_dFadeOutTime = 0.0;
	thread->AddThreadItem( &CCDAudio::_Play, trackNum, looping );
}

void CCDAudio::PlayFile( const char* pszFileName, bool looping )
{
	m_dFadeOutTime = 0.0;
	strncpy( m_szPendingPlayFilename, pszFileName, sizeof( m_szPendingPlayFilename ) );
	m_szPendingPlayFilename[ sizeof( m_szPendingPlayFilename ) - 1 ] = '\0';

	thread->AddThreadItem( &CCDAudio::_Play, 0, looping );

	std::this_thread::sleep_for( std::chrono::milliseconds( 0 ) );
}

void CCDAudio::Pause()
{
	if( m_bInitialized )
		thread->AddThreadItem( &CCDAudio::_Pause, 0, 0 );
}

void CCDAudio::Resume()
{
	if( m_bInitialized )
		thread->AddThreadItem( &CCDAudio::_Resume, 0, 0 );
}

void CCDAudio::Frame()
{
	if( m_bEnabled )
	{
		if( m_flVolume != bgmvolume.value )
		{
			if( m_flVolume == 0.0 )
			{
				m_flVolume = 1.0f;
				Resume();
			}
			else
			{
				m_flVolume = 0.0f;
				Pause();
			}

			Cvar_DirectSet( &bgmvolume, va( "%f", m_flVolume ) );
		}

		if( m_dFadeOutTime == 0.0 )
		{
			MP3_SetVolume( MP3Volume.value );
		}

		m_flMP3Volume = m_MP3.volume;

		thread->AddThreadItem( &CCDAudio::_CDUpdate, 0, 0 );
	}
}

void CCDAudio::MP3_Resume_Audio()
{
	MP3_Init();

	if( m_MP3.playing && m_MP3.suspended )
	{
		if( m_MP3.tracknum )
			MP3_PlayTrack( m_MP3.tracknum, m_MP3.looping );
		else
			MP3_PlayTrack( m_MP3.trackname, m_MP3.looping );
	}
}

void CCDAudio::MP3_Suspend_Audio()
{
	this->m_MP3.playing = m_bIsPlaying;

	if( m_bIsPlaying )
	{
		m_MP3.suspended = true;
	}

	MP3_StopStream();
	MP3_ReleaseDriver();
}

float CCDAudio::MP3_SetVolume( float newVol )
{
	if( newVol < 0.0f )
		newVol = 0;
	else if( newVol > 1.0f )
		newVol = 1.0f;

	if( m_MP3.volume != newVol )
	{
		m_MP3.volume = newVol;

		if( m_pStream )
		{
			m_pStream->setVolume( newVol );
		}
	}

	return newVol;
}

bool CCDAudio::MP3_PlayTrack( int trackNum, bool looping )
{
	if( trackNum > 1 && trackNum < g_iMP3NumTracks )
	{
		if( m_bIsPrimed || MP3_InitStream( trackNum, looping ) )
		{
			m_bIsPrimed = false;
			MP3_PlayTrackFinalize( trackNum, looping );
			return true;
		}
	}

	return false;
}

bool CCDAudio::MP3_PlayTrack( const char* filename, bool looping )
{
	if( m_bIsPrimed || MP3_InitStream( filename, looping ) )
	{
		m_bIsPrimed = false;
		MP3_PlayTrackFinalize( 0, looping );
		return true;
	}

	return false;
}

void CCDAudio::Eject()
{
	thread->AddThreadItem( &CCDAudio::_Eject, 0, 0 );
}

void CCDAudio::CloseDoor()
{
	thread->AddThreadItem( &CCDAudio::_CloseDoor, 0, 0 );
}

void CCDAudio::GetAudioDiskInfo()
{
	thread->AddThreadItem( &CCDAudio::_GetAudioDiskInfo, 0, 0 );
}

void CCDAudio::Reset()
{
	thread->AddThreadItem( &CCDAudio::_CDReset, 0, 0 );
}

void CCDAudio::Stop()
{
	thread->AddThreadItem( &CCDAudio::_Stop, 0, 0 );
}

void CCDAudio::SwitchToEngine()
{
	thread->AddThreadItem( &CCDAudio::_SwitchToEngine, 0, 0 );
}

void CCDAudio::SwitchToLauncher()
{
	thread->AddThreadItem( &CCDAudio::_SwitchToLauncher, 0, 0 );
}

void CCDAudio::ResetCDTimes()
{
	m_flPlayTime = 0;
	m_dStartTime = 0;
	m_dPauseTime = 0;
}

void CCDAudio::PrimeTrack( const char* filename, bool looping )
{
	m_dFadeOutTime = 0;

	strncpy( m_szPendingPlayFilename, filename, sizeof( m_szPendingPlayFilename ) );
	m_szPendingPlayFilename[ sizeof( m_szPendingPlayFilename ) - 1 ] = '\0';

	thread->AddThreadItem( &CCDAudio::_PrimeTrack, 0, looping );

	std::this_thread::sleep_for( std::chrono::milliseconds( 0 ) );

	if( m_bIsInMiddleOfPriming )
	{
		auto start = time( nullptr );
		decltype( start ) now;

		do
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 0 ) );
			now = time( nullptr );
		}
		while( m_bIsInMiddleOfPriming && ( now - start ) <= 1999 );
	}
}

void CCDAudio::FadeOut()
{
	if( m_bIsPlaying )
	{
		m_dFadeOutTime = MP3FadeTime.value + realtime;

		thread->AddThreadItem( &CCDAudio::_CDUpdate, 0, 0 );
	}
}

void CCDAudio::MP3_PlayTrackFinalize( int trackNum, bool looping )
{
	MP3_StartStream();

	m_MP3.inuse = true;
	m_flPlayTime = 0.0;

	m_MP3.playing = true;
	m_MP3.tracknum = trackNum;
	m_MP3.looping = looping;
	m_dStartTime = 0;
	m_dPauseTime = 0;
	m_bIsLooping = looping;
	m_nPlayTrack = trackNum;
	m_bIsPlaying = true;

	if( m_flMP3Volume == 0.0 && m_bEnabled )
	{
		m_bWasPlaying = true;
		if( m_pStream )
		{
			MP3_SetPause( true );
			m_bWasPlaying = m_bIsPlaying;
		}

		m_dPauseTime = realtime;
		m_bIsPlaying = false;
		m_szPendingPlayFilename[ 0 ] = '\0';
	}
}

void CCDAudio::CD_f()
{
	if( Cmd_Argc() <= 1 )
		return;

	const char* pszCommand = Cmd_Argv( 1 );

	if( !stricmp( pszCommand, "on" ) )
	{
		m_bEnabled = true;
	}
	if( !stricmp( pszCommand, "off" ) )
	{
		if( m_bIsPlaying )
			Stop();

		m_bEnabled = false;
	}
	else if( !stricmp( pszCommand, "reset" ) )
	{
		m_bEnabled = true;
		if( m_bIsPlaying )
			Stop();

		for( int i = 0; i < MAX_REMAP; ++i )
		{
			m_rgRemapCD[ i ] = i;
		}

		GetAudioDiskInfo();
	}
	else if( !stricmp( pszCommand, "remap" ) )
	{
		if( Cmd_Argc() > 2 )
		{
			const int iTotal = Cmd_Argc() - 1;

			for( int i = 2; i < iTotal; ++i )
			{
				m_rgRemapCD[ i ] = strtol( Cmd_Argv( i ), nullptr, 10 );
			}
		}
	}
	else if( !stricmp( pszCommand, "close" ) )
	{
		CloseDoor();
	}
	else if( !stricmp( pszCommand, "mp3info" ) )
	{
		Con_Printf( "Current MP3 Title: %s\n", m_MP3.trackname );
		Con_Printf( "Current MP3 Track: %i\n", m_MP3.tracknum );
		Con_Printf( "Current MP3 Volume: %i\n", m_MP3.volume );
	}
	else if( Cmd_Argc() > 2 && !stricmp( pszCommand, "mp3track" ) )
	{
		const int trackNum = strtol( Cmd_Argv( 2 ), 0, 10 ) + 1;

		if( MP3_PlayTrack( trackNum, false ) )
		{
			ResetCDTimes();
			m_bIsLooping = false;
			m_nPlayTrack = trackNum;
			m_bIsPlaying = true;
		}
	}
	else if( !stricmp( pszCommand, "play" ) )
	{
		Play( strtol( Cmd_Argv( 2 ), nullptr, 10 ), false );
	}
	else if( !stricmp( pszCommand, "playfile" ) )
	{
		PlayFile( Cmd_Argv( 2 ), false );
	}
	else if( !stricmp( pszCommand, "loop" ) )
	{
		Play( strtol( Cmd_Argv( 2 ), nullptr, 10 ), true );
	}
	else if( !stricmp( pszCommand, "loopfile" ) )
	{
		PlayFile( Cmd_Argv( 2 ), true );
	}
	else if( !stricmp( pszCommand, "stop" ) )
	{
		Stop();
	}
	else if( !stricmp( pszCommand, "fadeout" ) )
	{
		FadeOut();
	}
	else if( !stricmp( pszCommand, "pause" ) )
	{
		Pause();
	}
	else if( !stricmp( pszCommand, "resume" ) )
	{
		Resume();
	}
	else if( !stricmp( pszCommand, "eject" ) )
	{
		if( m_bIsPlaying )
			Stop();
		Eject();
		m_bIsCDValid = false;
	}
	else if( !stricmp( pszCommand, "info" ) )
	{
		Con_Printf( "%u tracks\n", g_iMP3NumTracks - 1 );
		if( m_bIsPlaying )
		{
			Con_Printf( "Currently %s track %u\n", m_bIsLooping ? "looping" : "playing", m_nPlayTrack );
		}
		else if( m_bWasPlaying )
		{
			Con_Printf( "Paused %s track %u\n", m_bIsLooping ? "looping" : "playing", m_nPlayTrack );
		}
		Con_Printf( "Volume is %f\n", m_flVolume );
	}
}

bool CCDAudio::MP3_Init()
{
	MP3_ReleaseDriver();

	FMOD_RESULT result = FMOD::System_Create( &m_pSystem );

	if( result != FMOD_OK )
	{
		std::cout << "MP3 startup failed due to " << FMOD_ErrorString( result ) << ", mp3 playback will not be available." << std::endl;
		return false;
	}

	//We only need 1 channel for the MP3.
	result = m_pSystem->init( 1, FMOD_INIT_NORMAL, nullptr );

	return true;
}

void CCDAudio::MP3_Shutdown()
{
	MP3_StopStream();
	MP3_ReleaseDriver();

	for( int i = g_iMP3FirstMalloc; i < MAX_MP3; ++i )
	{
		if( g_pszMP3trackFileMap[ i ] )
		{
			Mem_Free( const_cast<char*>( g_pszMP3trackFileMap[ i ] ) );
		}
	}
}

void CCDAudio::MP3_ReleaseDriver()
{
	if( m_pSystem )
	{
		m_pSystem->close();
		m_pSystem->release();
		m_pSystem = nullptr;
	}
}

void CCDAudio::MP3_StartStream()
{
	if( m_pSample )
		m_pSystem->playSound( m_pSample, nullptr, false, &m_pStream );
}

void CCDAudio::MP3_StopStream()
{
	if( m_pStream )
	{
		m_pStream->setPaused( true );
		m_pStream->stop();
		m_pStream = nullptr;

		if( m_pSample )
		{
			m_pSample->release();
			m_pSample = nullptr;
		}
	}

	m_bIsPrimed = false;
}

void CCDAudio::MP3_Loop( bool OnOff )
{
	if( m_pStream )
		m_pStream->setLoopCount( OnOff ? -1 : 0 );
}

void CCDAudio::MP3_SetPause( bool OnOff )
{
	if( m_pStream )
	{
		m_pStream->setPaused( OnOff );
	}
}

bool CCDAudio::MP3_InitStream( int trackNum, bool looping )
{
	bool bResult = false;

	m_bIsPrimed = false;

	if( m_pSystem || MP3_Init() )
	{
		char fullPath[ 512 ];

		const char* pszPath = g_pszMP3trackFileMap[ trackNum ];

		snprintf( fullPath, sizeof( fullPath ), "..\\%s", g_pszMP3trackFileMap[ trackNum ] );

		pszPath = fullPath;

		if( m_bEnabled && ( m_bIsPlaying || m_bWasPlaying ) )
		{
			m_bIsPlaying = false;
			m_bWasPlaying = false;
			m_szPendingPlayFilename[ 0 ] = '\0';

			MP3_StopStream();

			m_MP3.tracknum = 0;

			m_bIsPrimed = false;
			m_MP3.inuse = false;
			m_MP3.suspended = false;
			m_MP3.playing = false;
			m_MP3.trackname[ 0 ] = '\0';
			m_MP3.looping = false;
			m_MP3.volume = m_flMP3Volume;
			ResetCDTimes();
		}

		m_dFadeOutTime = 0;

		m_pSystem->createSound( pszPath, FMOD_2D | FMOD_LOOP_NORMAL | FMOD_3D_WORLDRELATIVE | FMOD_3D_INVERSEROLLOFF, nullptr, &m_pSample );

		if( m_pSample )
		{
			MP3_StartStream();

			if( m_pStream )
			{
				m_pStream->setVolume( m_MP3.volume );
				strcpy( m_MP3.trackname, g_pszMP3trackFileMap[ trackNum ] );

				MP3_Loop( looping );

				Con_DPrintf( "MP3_InitStream(%d, %s) successful\n", trackNum, g_pszMP3trackFileMap[ trackNum ] );
				return true;
			}
		}

		Con_DPrintf( "warning: MP3_InitStream(%d, %s) failed\n", trackNum, g_pszMP3trackFileMap[ trackNum ] );
	}

	return false;
}

bool CCDAudio::MP3_InitStream( const char* filename, bool looping )
{
	if( filename && *filename )
	{
		int trackNum;

		if( strstr( filename, ":" ) || strstr( filename, ".." ) )
			trackNum = 0;
		else
			trackNum = MP3_GetTrack( filename );

		return MP3_InitStream( trackNum, looping );
	}

	return false;
}

void CCDAudio::_Init( int, int )
{
	for( int iIndex = 0; iIndex < MAX_REMAP; ++iIndex )
	{
		m_rgRemapCD[ iIndex ] = iIndex;
	}

	thread->AddThreadItem( &CCDAudio::_GetAudioDiskInfo, 0, 0 );
}

void CCDAudio::_Eject( int, int )
{
	ResetCDTimes();
}

void CCDAudio::_CloseDoor( int, int )
{
	ResetCDTimes();
}

void CCDAudio::_GetAudioDiskInfo( int, int )
{
	m_bIsCDValid = false;
	m_nMaxCDTrack = 0;
}

void CCDAudio::_Play( int trackNum, int looping )
{
	if( m_bEnabled )
	{
		if( trackNum == 0 && m_szPendingPlayFilename[ 0 ] )
		{
			MP3_PlayTrack( m_szPendingPlayFilename, looping != 0 );
			m_szPendingPlayFilename[ 0 ] = '\0';
		}
		else
		{
			m_szPendingPlayFilename[ 0 ] = '\0';
			m_bIsCDValid = false;
			m_nMaxCDTrack = 0;
			MP3_PlayTrack( trackNum, looping != 0 );
		}
	}
}

void CCDAudio::_Pause( int, int )
{
	if( m_bEnabled && m_bIsPlaying )
	{
		m_bWasPlaying = true;

		if( m_MP3.inuse )
		{
			if( m_pStream )
			{
				MP3_SetPause( true );
				m_bWasPlaying = m_bIsPlaying;
			}
		}

		m_dPauseTime = realtime;
		m_bIsPlaying = false;
		m_szPendingPlayFilename[ 0 ] = '\0';
	}
}

void CCDAudio::_Resume( int, int )
{
	if( m_bEnabled && m_bIsPlaying )
	{
		m_bIsPlaying = true;

		if( !m_MP3.inuse )
		{
			double flDelta = realtime - m_dPauseTime;
			m_dPauseTime = 0;
			m_dStartTime += flDelta;
			return;
		}

		if( m_pStream )
		{
			MP3_SetPause( false );

			double flDelta = realtime - m_dPauseTime;
			m_dPauseTime = 0;
			m_dStartTime += flDelta;
		}
	}
}

void CCDAudio::_CDUpdate( int, int )
{
	if( m_bIsPlaying )
	{
		if( m_pSystem )
		{
			m_pSystem->update();
		}

		if( m_dFadeOutTime != 0.0 )
		{
			float flVolume;

			if( Sys_FloatTime() >= m_dFadeOutTime )
			{
				_Stop( 0, 0 );

				flVolume = MP3Volume.value;
				m_dFadeOutTime = 0;
			}
			else
			{
				flVolume = static_cast<float>( ( m_dFadeOutTime - Sys_FloatTime() ) / MP3FadeTime.value * MP3Volume.value );
			}

			MP3_SetVolume( flVolume );

			if( thread->AddThreadItem( &CCDAudio::_CDUpdate, 0, 0 ) )
				std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
		}

		if( m_MP3.inuse )
		{
			bool bIsPlaying;

			if( m_pStream != nullptr )
			{
				FMOD_RESULT result = m_pStream->isPlaying( &bIsPlaying );

				if( result == FMOD_ERR_INVALID_HANDLE || result == FMOD_ERR_CHANNEL_STOLEN )
					bIsPlaying = false;
			}
			else
				bIsPlaying = false;

			m_bIsPlaying = bIsPlaying;

			if( !bIsPlaying )
			{
				m_szPendingPlayFilename[ 0 ] = '\0';
			}
		}

		if( m_flPlayTime != 0.0 && m_dStartTime != 0.0 )
		{
			if( ( realtime - m_dStartTime ) >= m_flPlayTime + 2.0 )
			{
				if( m_bIsPlaying )
				{
					m_bIsPlaying = false;
					m_szPendingPlayFilename[ 0 ] = '\0';
					if( m_bIsLooping )
					{
						Play( m_nPlayTrack, true );
					}
				}
			}
		}
	}
}

void CCDAudio::_CDReset( int, int )
{
	Shutdown();
	std::this_thread::sleep_for( std::chrono::milliseconds( 0 ) );
	Init();
}

void CCDAudio::_Stop( int, int )
{
	if( m_bEnabled && ( m_bIsPlaying || m_bWasPlaying ) )
	{
		m_bIsPlaying = false;
		m_bWasPlaying = false;
		m_szPendingPlayFilename[ 0 ] = '\0';

		MP3_StopStream();

		m_MP3.tracknum = 0;

		m_MP3.inuse = false;
		m_MP3.suspended = false;
		m_MP3.playing = false;
		m_MP3.trackname[ 0 ] = '\0';
		m_MP3.looping = false;
		m_MP3.volume = m_flMP3Volume;
		ResetCDTimes();
	}
}

void CCDAudio::_SwitchToEngine( int, int )
{
	if( m_bResumeOnSwitch )
	{
		m_bResumeOnSwitch = false;

		if( m_bEnabled )
		{
			if( m_bWasPlaying )
			{
				if( !m_MP3.inuse )
				{
					m_bIsPlaying = true;
					m_dPauseTime = 0;
					m_dStartTime += ( realtime - m_dPauseTime );
					return;
				}
				if( m_pStream )
				{
					MP3_SetPause( false );
					m_bIsPlaying = true;

					if( !m_MP3.inuse )
					{
						m_dPauseTime = 0;
						m_dStartTime += ( realtime - m_dPauseTime );
					}
				}
				else
				{
					m_bIsPlaying = true;
				}
			}
		}
	}
}

void CCDAudio::_SwitchToLauncher( int, int )
{
	if( m_bEnabled && m_bIsPlaying )
	{
		m_bWasPlaying = true;
		m_bResumeOnSwitch = true;

		if( m_MP3.inuse )
		{
			if( m_pStream )
			{
				MP3_SetPause( true );
				m_bWasPlaying = m_bIsPlaying;
			}
		}

		m_dPauseTime = realtime;
		m_bIsPlaying = false;
		m_szPendingPlayFilename[ 0 ] = '\0';
	}
}

void CCDAudio::_PrimeTrack( int track, int looping )
{
	m_bIsInMiddleOfPriming = true;
	if( track )
	{
		if( track > 1 && track < g_iMP3NumTracks )
		{
			MP3_InitStream( track, looping != 0 );
			m_bIsPrimed = true;
		}
	}
	else if( m_szPendingPlayFilename[ 0 ] )
	{
		MP3_InitStream( m_szPendingPlayFilename, looping != 0 );
		m_bIsPrimed = true;
	}
	m_bIsInMiddleOfPriming = false;
}
