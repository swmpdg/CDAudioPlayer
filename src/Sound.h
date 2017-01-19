#ifndef SOUND_H
#define SOUND_H

/**
*	@file
*
*	Sound system interface.
*/

#ifdef __cplusplus
#include "CCDAudio.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

void S_Init( void );

void CDAudio_Init( void );
void CDAudio_Shutdown( void );

void CDAudio_Play( int track, int looping );

void CDAudio_Pause( void );

void CDAudio_Resume( void );

void CD_Command_f( void );

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern ICDAudio* cdaudio;

CCDAudio* GetInteralCDAudio();
#endif

#endif //SOUND_H
