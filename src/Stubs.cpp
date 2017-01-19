#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "Stubs.h"

double realtime = 0.0;

void Cvar_Register( cvar_t* pCVar )
{
	pCVar->string = Mem_Strdup( pCVar->string );
}

void Cvar_DirectSet( cvar_t* pCVar, const char* pszValue )
{
	char* pszOld = pCVar->string;

	pCVar->string = Mem_Strdup( pszValue );
	pCVar->value = static_cast<float>( atof( pszValue ) );

	//Free this after setting in case pszValue is actually this string.
	Mem_Free( pszOld );
}

char* Mem_Strdup( const char* pszString )
{
	const size_t uiLength = strlen( pszString );

	char* pszDest = new char[ uiLength + 1 ];

	strcpy( pszDest, pszString );

	return pszDest;
}

void Mem_Free( void* pMem )
{
	delete[] pMem;
}

char* COM_FixSlashes( char* pszString )
{
	for( char* psz = pszString; *psz; ++psz )
	{
		if( *psz == '\\' )
			*psz = '/';
	}

	return pszString;
}

const char* va( const char* pszFormat, ... )
{
	static char szBuffer[ 4096 ];

	va_list list;

	va_start( list, pszFormat );

	vsnprintf( szBuffer, sizeof( szBuffer ), pszFormat, list );

	va_end( list );

	return szBuffer;
}
