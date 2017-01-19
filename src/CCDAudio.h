#ifndef CCDAUDIO_H
#define CCDAUDIO_H

#include <vector>

#include <fmod.hpp>

#include "ICDAudio.h"

/**
*	CD Audio system.
*/
class CCDAudio final : public ICDAudio
{
public:
	struct _MP3_AUDIO_STATE
	{
		bool inuse;
		bool suspended;
		bool playing;
		char trackname[ 512 ];
		int tracknum;
		bool looping;
		float volume;
	};

	enum
	{
		MAX_REMAP = 100
	};

	enum
	{
		MP3_PAUSE = 1,
		MP3_RESUME = 0
	};

public:
	CCDAudio() = default;
	~CCDAudio() = default;

	bool Init() override;

	void Shutdown() override;

	void Play( int trackNum, bool looping ) override;

	void PlayFile( const char* pszFileName, bool looping ) override;

	void Pause() override;

	void Resume() override;

	void Frame() override;

	void MP3_Resume_Audio() override;

	void MP3_Suspend_Audio() override;

	float MP3_SetVolume( float newVol );

	bool MP3_PlayTrack( int trackNum, bool looping );

	bool MP3_PlayTrack( const char* filename, bool looping );

	void Eject();

	void CloseDoor();

	void GetAudioDiskInfo();

	void Reset();

	void Stop();

	void SwitchToEngine();

	void SwitchToLauncher();

	void ResetCDTimes();

	void PrimeTrack( const char* filename, bool looping );

	void FadeOut();

	void MP3_PlayTrackFinalize( int trackNum, bool looping );

	void CD_f();

private:
	bool MP3_Init();

	void MP3_Shutdown();

	void MP3_ReleaseDriver();

	void MP3_StartStream();

	void MP3_StopStream();

	void MP3_Loop( bool OnOff );

	void MP3_SetPause( bool OnOff );

	bool MP3_InitStream( int trackNum, bool looping );

	bool MP3_InitStream( const char* filename, bool looping );

	void _Init( int, int );

	void _Eject( int, int );

	void _CloseDoor( int, int );

	void _GetAudioDiskInfo( int, int );

	void _Play( int trackNum, int looping );

	void _Pause( int, int );

	void _Resume( int, int );

	void _CDUpdate( int, int );

	void _CDReset( int, int );

	void _Stop( int, int );

	void _SwitchToEngine( int, int );

	void _SwitchToLauncher( int, int );

	void _PrimeTrack( int track, int looping );

private:
	float m_flPlayTime = 0;
	double m_dStartTime = 0;
	double m_dPauseTime = 0;
	double m_dFadeOutTime = 0;

	bool m_bIsCDValid = false;
	bool m_bIsPlaying = false;
	bool m_bWasPlaying = false;
	bool m_bInitialized = false;
	bool m_bEnabled = false;
	bool m_bIsLooping = false;
	bool m_bIsPrimed = false;
	volatile bool m_bIsInMiddleOfPriming = false;

	float m_flVolume = 1.0f;
	float m_flMP3Volume = 1.0f;

	int m_nPlayTrack = 0;
	char m_szPendingPlayFilename[ 512 ] = {};
	int m_nMaxCDTrack = 0;

	bool m_bResumeOnSwitch = 0;

	int m_rgRemapCD[ MAX_REMAP ] = {};

	_MP3_AUDIO_STATE m_MP3;

	FMOD::System* m_pSystem = nullptr;
	FMOD::Sound* m_pSample = nullptr;
	FMOD::Channel* m_pStream = nullptr;
};

#endif //CCDAUDIO_H
