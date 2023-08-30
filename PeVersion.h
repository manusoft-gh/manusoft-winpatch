// Copyright 2010 ManuSoft
// https://www.manusoft.com
// All Rights Reserved
//
// Use of this software is governed by the terms of:
// GNU General Public License v2.0
// See LICENSE for details

#pragma once

#ifndef _PEVERSION_H_
#define _PEVERSION_H_


#include "PeUtility.h"

#ifndef DLLEXPORT
#define DLLEXPORT
#endif


class PeFileVersion
{
	WORD m_wMajor;
	WORD m_wMinor;
	WORD m_wThird;
	WORD m_wFourth;

protected:
	DLLEXPORT void InitializeA( LPCSTR pszModulePath );
	DLLEXPORT void InitializeW( LPCWSTR pszModulePath );

public:
	PeFileVersion( WORD wMajor = 0, WORD wMinor = 0, WORD wThird = 0, WORD wFourth = 0 )
	: m_wMajor( wMajor ), m_wMinor( wMinor ), m_wThird( wThird ), m_wFourth( wFourth ) {}

	PeFileVersion( HMODULE hmodTarget )
	: m_wMajor( 0 ), m_wMinor( 0 ), m_wThird( 0 ), m_wFourth( 0 )
	{
		InitializeA( PeGetModulePathA( hmodTarget ) );
	}

	PeFileVersion( LPCSTR pszModulePath )
	: m_wMajor( 0 ), m_wMinor( 0 ), m_wThird( 0 ), m_wFourth( 0 )
	{
		InitializeA( pszModulePath );
	}

	PeFileVersion( LPCWSTR pszModulePath )
	: m_wMajor( 0 ), m_wMinor( 0 ), m_wThird( 0 ), m_wFourth( 0 )
	{
		InitializeW( pszModulePath );
	}

	PeFileVersion& operator =( const PeFileVersion& Ver )
	{
		m_wMajor = Ver.m_wMajor;
		m_wMinor = Ver.m_wMinor;
		m_wThird = Ver.m_wThird;
		m_wFourth = Ver.m_wFourth;
		return *this;
	}

	WORD major() const { return m_wMajor; }
	WORD minor() const { return m_wMinor; }
	WORD third() const { return m_wThird; }
	WORD fourth() const { return m_wFourth; }

	bool operator ==( const PeFileVersion& Ver )
	{ return (m_wMajor == Ver.m_wMajor &&
						m_wMinor == Ver.m_wMinor &&
						m_wThird == Ver.m_wThird &&
						m_wFourth == Ver.m_wFourth);
	}

	bool operator >=( const PeFileVersion& Ver )
	{ if( m_wMajor < Ver.m_wMajor )
			return false;
		else if( m_wMajor > Ver.m_wMajor )
			return true;
		if( m_wMinor < Ver.m_wMinor )
			return false;
		else if( m_wMinor > Ver.m_wMinor )
			return true;
		if( m_wThird < Ver.m_wThird )
			return false;
		else if( m_wThird > Ver.m_wThird )
			return true;
		if( m_wFourth < Ver.m_wFourth )
			return false;
		return true;
	}

	bool operator <=( const PeFileVersion& Ver )
	{ if( m_wMajor > Ver.m_wMajor )
			return false;
		else if( m_wMajor < Ver.m_wMajor )
			return true;
		if( m_wMinor > Ver.m_wMinor )
			return false;
		else if( m_wMinor < Ver.m_wMinor )
			return true;
		if( m_wThird > Ver.m_wThird )
			return false;
		else if( m_wThird < Ver.m_wThird )
			return true;
		if( m_wFourth > Ver.m_wFourth )
			return false;
		return true;
	}

	bool operator !=( const PeFileVersion& Ver ) { return !(*this == Ver); }

	bool operator <( const PeFileVersion& Ver ) { return !(*this >= Ver); }

	bool operator >( const PeFileVersion& Ver ) { return !(*this <= Ver); }
};

#endif //_PEVERSION_H_
