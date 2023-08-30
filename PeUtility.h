// Copyright 2010 ManuSoft
// https://www.manusoft.com
// All Rights Reserved
//
// Use of this software is governed by the terms of:
// GNU General Public License v2.0
// See LICENSE for details

#pragma once

#ifndef _PEUTILITY_H_
#define _PEUTILITY_H_


#include "PeImage.h"

#ifndef DLLEXPORT
#define DLLEXPORT
#endif

//#define wchar_t
typedef wchar_t const * LPCWSTR;

#ifdef _WIN64
typedef unsigned __int64 ORDINAL;
#else
typedef unsigned __int32 ORDINAL;
#endif //_WIN64

//Get memory address from HMODULE (also works in Win32s)
DLLEXPORT PIMAGE_DOS_HEADER PeGetModuleBase( HMODULE hmodTarget );

inline LPCSTR PeGetModulePathA( HMODULE hmodTarget )
{
	static CHAR szPath[MAX_PATH + 1];
	if( ::GetModuleFileNameA( hmodTarget, szPath, MAX_PATH ) == 0 )
		return NULL;
	return szPath;
}

inline LPCWSTR PeGetModulePathW( HMODULE hmodTarget )
{
	static WCHAR szPath[MAX_PATH + 1];
	if( ::GetModuleFileNameW( hmodTarget, szPath, MAX_PATH ) == 0 )
	{
		CHAR szaPath[MAX_PATH + 1];
		if( ::GetModuleFileNameA( hmodTarget, szaPath, MAX_PATH ) == 0 )
			return NULL;
		if( MultiByteToWideChar( CP_ACP, 0, szaPath, -1, szPath, MAX_PATH ) == 0 )
			return NULL;
	}
	return szPath;
}

inline HMODULE PeGetModuleFromAddress( void const* addr )
{
	MEMORY_BASIC_INFORMATION mbi;
	if( !::VirtualQuery( addr, &mbi, sizeof(mbi) ) )
		return NULL;
	return (HMODULE)mbi.AllocationBase;
}

DLLEXPORT LPVOID PeGetPreferredBaseAddress( const PeImage& Target );
DLLEXPORT LPCSTR PeGetModulePathA( const PeImage& Target );
DLLEXPORT LPCWSTR PeGetModulePathW( const PeImage& Target );
DLLEXPORT LPCSTR PeGetModuleDirectoryA( const PeImage& Target );
DLLEXPORT LPCWSTR PeGetModuleDirectoryW( const PeImage& Target );
DLLEXPORT LPCSTR PeGetModuleFilenameA( const PeImage& Target );
DLLEXPORT LPCWSTR PeGetModuleFilenameW( const PeImage& Target );
DLLEXPORT bool PeModuleDependentOn( const PeImage& TestApp, LPCSTR pszTestModule );
DLLEXPORT bool PeModuleImplicitDependentOn( const PeImage& TestApp, LPCSTR pszTestModule );
DLLEXPORT bool PeModuleDelayDependentOn( const PeImage& TestApp, LPCSTR pszTestModule );
DLLEXPORT FARPROC* PeGetImportTableEntry( const PeImage& ImportingModule,
																					LPCSTR pszExportingModule,
																					LPCSTR pszFunction );
inline FARPROC* PeGetImportTableEntry( const PeImage& ImportingModule,
																			 LPCSTR pszExportingModule,
																			 DWORD_PTR dwFunctionOrdinal )
	{ return PeGetImportTableEntry( ImportingModule, pszExportingModule, (LPCSTR)dwFunctionOrdinal ); }
DLLEXPORT FARPROC* PeGetImplicitImportTableEntry( const PeImage& ImportingModule,
																									LPCSTR pszExportingModule,
																									LPCSTR pszFunction );
inline FARPROC* PeGetImplicitImportTableEntry( const PeImage& ImportingModule,
																							 LPCSTR pszExportingModule,
																							 DWORD_PTR dwFunctionOrdinal )
	{ return PeGetImplicitImportTableEntry( ImportingModule, pszExportingModule, (LPCSTR)dwFunctionOrdinal ); }
DLLEXPORT FARPROC* PeGetDelayLoadImportTableEntry( const PeImage& ImportingModule,
																									 LPCSTR pszExportingModule,
																									 LPCSTR pszFunction );
inline FARPROC* PeGetDelayLoadImportTableEntry( const PeImage& ImportingModule,
																								LPCSTR pszExportingModule,
																								DWORD_PTR dwFunctionOrdinal )
	{ return PeGetDelayLoadImportTableEntry( ImportingModule, pszExportingModule, (LPCSTR)dwFunctionOrdinal ); }
DLLEXPORT LONG PeSearchExportNameTable( const PeImage& ExportingModule, LPCSTR pszFunction );
DLLEXPORT RVA* PeGetExportTableEntry( const PeImage& ExportingModule, LPCSTR pszFunction );
DLLEXPORT RVA* PeGetExportTableEntry( const PeImage& ExportingModule, ORDINAL dwOrdinal );
DLLEXPORT FARPROC PeGetProcAddress( const PeImage& ExportingModule, LPCSTR pszFunction );
inline FARPROC PeGetProcAddress( const PeImage& ExportingModule, ORDINAL dwFunctionOrdinal )
	{ return PeGetProcAddress( ExportingModule, (LPCSTR)dwFunctionOrdinal ); }

//define character neutral versions
#ifdef UNICODE
	#define PeGetModulePath PeGetModulePathW
	#define PeGetModuleDirectory PeGetModuleDirectoryW
	#define PeGetModuleFilename PeGetModuleFilenameW
#else
	#define PeGetModulePath PeGetModulePathA
	#define PeGetModuleDirectory PeGetModuleDirectoryA
	#define PeGetModuleFilename PeGetModuleFilenameA
#endif

//Enumerate all base relocations
typedef bool (*ENUMBASERELOCATIONCB)( class PeBaseRelocation const& Reloc );
DLLEXPORT bool EnumerateBaseRelocations( PeImage& PeSource, ENUMBASERELOCATIONCB pfCallback );

inline
bool IsOrdinal( LPCSTR pszFunction )
{
	return ((LPCSTR)IMAGE_ORDINAL32((ORDINAL)pszFunction) == pszFunction);
}

DLLEXPORT bool CompareNameOrdinal( LPCSTR pszFunction1, LPCSTR pszFunction2 );

template <typename FUNC>
class ImportedFunction
{
	HMODULE m_hmodTarget;
	FUNC m_pfTarget;
	bool m_bMustFree;
public:
	ImportedFunction( LPCSTR lpszModule, LPCSTR lpszFunction )
		: m_hmodTarget( ::LoadLibraryA( lpszModule ) ),
			m_pfTarget( NULL ),
			m_bMustFree( (m_hmodTarget != NULL) )
		{
			if( m_hmodTarget )
			{
				m_pfTarget = (FUNC)::GetProcAddress( m_hmodTarget, lpszFunction );
				if( !m_pfTarget )
				{
					::FreeLibrary( m_hmodTarget );
					m_bMustFree = false;
				}
			}
		}
	ImportedFunction( LPCSTR lpszModule, ORDINAL nOrdinal )
		: m_hmodTarget( ::LoadLibraryA( lpszModule ) ),
			m_pfTarget( NULL ),
			m_bMustFree( (m_hmodTarget != NULL) )
		{
			if( m_hmodTarget )
			{
				m_pfTarget = (FUNC)::GetProcAddress( m_hmodTarget, (LPCSTR)nOrdinal );
				if( !m_pfTarget )
				{
					::FreeLibrary( m_hmodTarget );
					m_bMustFree = false;
				}
			}
		}
	ImportedFunction( LPCWSTR lpszModule, LPCSTR lpszFunction )
		: m_hmodTarget( ::LoadLibraryW( lpszModule ) ),
			m_pfTarget( NULL ),
			m_bMustFree( (m_hmodTarget != NULL) )
		{
			if( m_hmodTarget )
			{
				m_pfTarget = (FUNC)::GetProcAddress( m_hmodTarget, lpszFunction );
				if( !m_pfTarget )
				{
					::FreeLibrary( m_hmodTarget );
					m_bMustFree = false;
				}
			}
		}
	ImportedFunction( LPCWSTR lpszModule, ORDINAL nOrdinal )
		: m_hmodTarget( ::LoadLibraryW( lpszModule ) ),
			m_pfTarget( NULL ),
			m_bMustFree( (m_hmodTarget != NULL) )
		{
			if( m_hmodTarget )
			{
				m_pfTarget = (FUNC)::GetProcAddress( m_hmodTarget, (LPCSTR)nOrdinal );
				if( !m_pfTarget )
				{
					::FreeLibrary( m_hmodTarget );
					m_bMustFree = false;
				}
			}
		}
	ImportedFunction( HMODULE hmodModule, LPCSTR lpszFunction ) //hmodModule cannot be NULL
		: m_hmodTarget( hmodModule ),
			m_pfTarget( (FUNC)::GetProcAddress( m_hmodTarget, lpszFunction ) ),
			m_bMustFree( false )
		{}
	ImportedFunction( HMODULE hmodModule, ORDINAL nOrdinal ) //hmodModule cannot be NULL
		: m_hmodTarget( hmodModule ),
			m_pfTarget( (FUNC)::GetProcAddress( m_hmodTarget, (LPCSTR)nOrdinal ) ),
			m_bMustFree( false )
		{}
	ImportedFunction( LPCSTR lpszFunction ) //Assume import from process module
		: m_hmodTarget( NULL ),
			m_pfTarget( (FUNC)::GetProcAddress( ::GetModuleHandle( NULL ), lpszFunction ) ),
			m_bMustFree( false )
		{}
	ImportedFunction( ORDINAL nOrdinal ) //Assume import from process module
		: m_hmodTarget( NULL ),
			m_pfTarget( (FUNC)::GetProcAddress( ::GetModuleHandle( NULL ), (LPCSTR)nOrdinal ) ),
			m_bMustFree( false )
		{}
	~ImportedFunction() { if( m_bMustFree ) ::FreeLibrary( m_hmodTarget ); }
	HMODULE GetLib() const { return m_hmodTarget; }
	FUNC GetProc() const { return m_pfTarget; }
	operator bool() const { return (m_pfTarget != NULL); }
};


class PeBaseRelocation
{
	PeImage& m_PeSource;
	PIMAGE_BASE_RELOCATION m_pRelocTable;
	const WORD* m_pReloc;
public:
	PeBaseRelocation( PeImage& PeSource, const PIMAGE_BASE_RELOCATION pRelocTable, const WORD* pReloc )
		:	m_PeSource( PeSource ),
			m_pRelocTable( pRelocTable ),
			m_pReloc( pReloc )
		{}
	PeBaseRelocation& operator=( const PeBaseRelocation& src )
		{
			m_PeSource = src.m_PeSource;
			m_pRelocTable = src.m_pRelocTable;
			m_pReloc = src.m_pReloc;
			return *this;
		}
	RVA TargetRva() { return m_pRelocTable->VirtualAddress + (*m_pReloc & 0xfff); }
	LPVOID Target() { return m_PeSource.TranslateRva( TargetRva() ); }
	USHORT Type() { return USHORT(*m_pReloc >> 12); }
};


class ModuleBaseRelocations
{
	PeImage& m_PeSource;
	ULONG m_cbRelocSize;
	PIMAGE_BASE_RELOCATION m_pRelocBlock;
public:
	ModuleBaseRelocations( PeImage& PeSource )
		:	m_PeSource( PeSource ),
			m_cbRelocSize( 0 ),
			m_pRelocBlock( PeSource.GetBaseRelocationDirectory( m_cbRelocSize ) )
		{}
	ModuleBaseRelocations& operator=( const ModuleBaseRelocations& src )
		{
			m_PeSource = src.m_PeSource;
			m_cbRelocSize = src.m_cbRelocSize;
			m_pRelocBlock = src.m_pRelocBlock;
			return *this;
		}
	PIMAGE_BASE_RELOCATION GetRelocationTable() const { return m_pRelocBlock; }
	ULONG GetRelocationTableSize() const { return m_cbRelocSize; }
	operator PIMAGE_BASE_RELOCATION() const { return GetRelocationTable(); }
};

#endif //_PEUTILITY_H_
