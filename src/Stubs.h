#ifndef STUBS_H
#define STUBS_H

/**
*	@file
*
*	Implements stubs for engine functions.
*/

#include <cstdlib>

extern double realtime;

inline int Cmd_Argc() { return 0; }

inline const char* Cmd_Argv( int ) { return nullptr; }

inline void Con_Printf( const char* pszFormat, ... )
{
}

inline void Con_DPrintf( const char* pszFormat, ... )
{
}

inline float Sys_FloatTime()
{
	return static_cast<float>( realtime );
}

struct cvar_t
{
	const char* name;
	char* string;
	int flags;
	float value;
	cvar_t* next;

	cvar_t( const char* name, const char* string, int flags = 0 )
		: name( name )
		, string( const_cast<char*>( string ) )
		, flags( flags )
		, value( static_cast<float>( atof( string ) ) )
		, next( nullptr )
	{
	}
};

void Cvar_Register( cvar_t* pCVar );

void Cvar_DirectSet( cvar_t* pCVar, const char* pszValue );

char* Mem_Strdup( const char* pszString );

void Mem_Free( void* pMem );

char* COM_FixSlashes( char* pszString );

const char* va( const char* pszFormat, ... );

#ifdef WIN32
#define stricmp _stricmp
#endif

#endif //STUBS_H
