// Copyright 2010 ManuSoft
// https://www.manusoft.com
// All Rights Reserved
//
// Use of this software is governed by the terms of:
// GNU General Public License v2.0
// See LICENSE for details

#include "StdAfx.h"
#include "MemIO.h"
#include "PeUtility.h"
#include "PeImage.h"
#include "ExportedFunctionHook.h"


ExportedFunctionHook::ExportedFunctionHook( const LPCTSTR pszTargetModule,
																						const LPCSTR pszFunction,
																						FARPROC pHookFunc )
:	m_pThisHook( NULL ),
	m_hmodTarget( ::GetModuleHandle( pszTargetModule ) ),
	m_pfOriginalFunction( NULL ),
	m_pfHookFunction( pHookFunc )
{
	Initialize( pszFunction, pHookFunc );
}

ExportedFunctionHook::ExportedFunctionHook( const PeImage& PeTarget,
																						const LPCSTR pszFunction,
																						FARPROC pHookFunc )
:	m_pThisHook( NULL ),
	m_hmodTarget( PeTarget ),
	m_pfOriginalFunction( NULL ),
	m_pfHookFunction( pHookFunc )
{
	Initialize( pszFunction, pHookFunc );
}


ExportedFunctionHook::ExportedFunctionHook( const LPCTSTR pszTargetModule,
																						const DWORD dwFunctionOrdinal,
																						FARPROC pHookFunc )
:	m_pThisHook( NULL ),
	m_hmodTarget( ::GetModuleHandle( pszTargetModule ) ),
	m_pfOriginalFunction( NULL ),
	m_pfHookFunction( pHookFunc )
{
	Initialize( (LPCSTR)(DWORD_PTR)dwFunctionOrdinal, pHookFunc );
}


ExportedFunctionHook::ExportedFunctionHook( const PeImage& PeTarget,
																						const DWORD dwFunctionOrdinal,
																						FARPROC pHookFunc )
:	m_pThisHook( NULL ),
	m_hmodTarget( PeTarget ),
	m_pfOriginalFunction( NULL ),
	m_pfHookFunction( pHookFunc )
{
	Initialize( (LPCSTR)(DWORD_PTR)dwFunctionOrdinal, pHookFunc );
}


bool ExportedFunctionHook::Initialize( const LPCSTR pszFunction,
																			 FARPROC pHookFunc )
{
	if( !m_hmodTarget || !pHookFunc )
		return false;
	if( ((WORD)(DWORD_PTR)pszFunction != (DWORD_PTR)pszFunction && pszFunction[0] == '\0') )
		return false;

	PeImage PeTarget( m_hmodTarget );

	m_pfOriginalFunction = ::GetProcAddress( m_hmodTarget, pszFunction );
	RVA* pdwHook = PeGetExportTableEntry( PeTarget, pszFunction );
	if( !pdwHook )
		return false;

	// Put the hook patch in place
	m_pThisHook = new PatchT<RVA>( pdwHook, (RVA)((BYTE*)pHookFunc - (BYTE*)PeTarget.GetActualBaseAddress()) );
	if( !m_pThisHook )
		return false;

	return true;
}


ExportedFunctionHook::~ExportedFunctionHook()
{
	try
	{
		delete m_pThisHook;
	}
	catch( ... ) {}
}
