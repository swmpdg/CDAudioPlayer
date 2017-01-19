#ifndef ICDAUDIO_H
#define ICDAUDIO_H

/**
*	Interface for the CD Audio system.
*/
class ICDAudio
{
public:
	virtual bool Init() = 0;

	virtual void Shutdown() = 0;

	virtual void Play( int trackNum, bool looping ) = 0;

	virtual void PlayFile( const char* pszFileName, bool looping ) = 0;

	virtual void Pause() = 0;

	virtual void Resume() = 0;

	virtual void Frame() = 0;

	virtual void MP3_Resume_Audio() = 0;

	virtual void MP3_Suspend_Audio() = 0;

	virtual ~ICDAudio() = 0;
};

inline ICDAudio::~ICDAudio()
{
}

#endif //ICDAUDIO_H
