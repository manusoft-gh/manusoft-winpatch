// Copyright 2010 ManuSoft
// https://www.manusoft.com
// All Rights Reserved
//
// Use of this software is governed by the terms of:
// GNU General Public License v2.0
// See LICENSE for details

#include "StdAfx.h"
#include "PeUtility.h"
#include "MemIO.h"


static inline int inline_strcmp( const char* pszString1, const char* pszString2 )
{
	const char* pszCursor1 = pszString1;
	const char* pszCursor2 = pszString2;
	int nCmp;
	while ((nCmp = *pszCursor1 - *pszCursor2) == 0)
	{
		if( !*pszCursor1 )
			break;
		++pszCursor1;
		++pszCursor2;
	}
	return nCmp;
}

// Converts an HMODULE under Win32s to a base address in memory
PIMAGE_DOS_HEADER PeGetModuleBase( HMODULE hmodTarget )
{
#ifdef _WIN64
	return (PIMAGE_DOS_HEADER)((DWORD_PTR)hmodTarget & ((DWORD_PTR)-1 << 2));
#else
	static bool bWin32s = ((::GetVersion() & 0xC0000000) == 0x80000000);
	if( !bWin32s )
		return (PIMAGE_DOS_HEADER)((DWORD_PTR)hmodTarget & ((DWORD_PTR)-1 << 2));

	static HMODULE hmodW32S = ::GetModuleHandleA( "W32SKRNL.DLL" );
	typedef DWORD (__stdcall *XPROC)(DWORD);
	static XPROC ImteFromHModule = (XPROC)::GetProcAddress( hmodW32S, "_ImteFromHModule@4" );
	static XPROC BaseAddrFromImte = (XPROC)::GetProcAddress( hmodW32S, "_BaseAddrFromImte@4" );

	if( !ImteFromHModule || !BaseAddrFromImte )
		return NULL;

	DWORD imte = ImteFromHModule( (DWORD)hmodTarget );
	if( 0 == imte )
		return NULL;

	return (PIMAGE_DOS_HEADER)BaseAddrFromImte( imte );
#endif
}


LPVOID PeGetPreferredBaseAddress( const PeImage& Target )
{
	return (LPVOID)Target.GetPreferredBaseAddress();
}


LPCSTR PeGetModulePathA( const PeImage& Target )
{
	return Target.IsMappedFile()?
		Target.GetMappedFile()->GetModulePath() :
		PeGetModulePathA( (HMODULE)Target );
}


LPCWSTR PeGetModulePathW( const PeImage& Target )
{
	if( !Target.IsMappedFile() )
		return PeGetModulePathW( (HMODULE)Target );
	static WCHAR szPath[MAX_PATH + 1];
	if( 0 != MultiByteToWideChar( CP_ACP,
																0,
																Target.GetMappedFile()->GetModulePath(),
																-1,
																szPath,
																MAX_PATH ) )
		return szPath;
	return NULL;
}


LPCSTR PeGetModuleDirectoryA( const PeImage& Target )
{
	static CHAR szDirectory[MAX_PATH + 1];
	const CHAR* pszPath = PeGetModulePathA( Target );
	if( NULL == pszPath )
		return NULL;
	CHAR* pszCursor = szDirectory;
	CHAR* pszLastSeparator = NULL;
	do
	{
		*pszCursor = *pszPath++;
		if( *pszCursor == '\\' || *pszCursor == '/' )
			pszLastSeparator = pszCursor;
	}
	while( *pszCursor++ );
	if( !pszLastSeparator )
		return NULL; //error, should always be at least one separator
	*++pszLastSeparator = '\0';
	return szDirectory;
}


LPCWSTR PeGetModuleDirectoryW( const PeImage& Target )
{
	static WCHAR szDirectory[MAX_PATH + 1];
	const WCHAR* pszPath = PeGetModulePathW( Target );
	if( NULL == pszPath )
		return NULL;
	WCHAR* pszCursor = szDirectory;
	WCHAR* pszLastSeparator = NULL;
	do
	{
		*pszCursor = *pszPath++;
		if( *pszCursor == L'\\' || *pszCursor == L'/' )
			pszLastSeparator = pszCursor;
	}
	while( *pszCursor++ );
	if( !pszLastSeparator )
		return NULL; //error, should always be at least one separator
	*++pszLastSeparator = L'\0';
	return szDirectory;
}


LPCSTR PeGetModuleFilenameA( const PeImage& Target )
{
	static CHAR szFilename[MAX_PATH + 1];
	const CHAR* pszPath = PeGetModulePathA( Target );
	if( NULL == pszPath )
		return NULL;
	const CHAR* pszLastSeparator = NULL;
	do
	{
		if( *pszPath == '\\' || *pszPath == '/' )
			pszLastSeparator = pszPath;
	}
	while( *pszPath++ );
	if( !pszLastSeparator )
		return NULL; //error, should always be at least one separator
	CHAR* pszCursor = &szFilename[0];
	for( pszLastSeparator++; *pszLastSeparator != '\0'; *pszCursor++ = *pszLastSeparator++ );
	*pszCursor = '\0';
	return szFilename;
}


LPCWSTR PeGetModuleFilenameW( const PeImage& Target )
{
	static WCHAR szFilename[MAX_PATH + 1];
	const WCHAR* pszPath = PeGetModulePathW( Target );
	if( NULL == pszPath )
		return NULL;
	const WCHAR* pszLastSeparator = NULL;
	do
	{
		if( *pszPath == L'\\' || *pszPath == L'/' )
			pszLastSeparator = pszPath;
	}
	while( *pszPath++ );
	if( !pszLastSeparator )
		return NULL; //error, should always be at least one separator
	WCHAR* pszCursor = &szFilename[0];
	for( pszLastSeparator++; *pszLastSeparator != L'\0'; *pszCursor++ = *pszLastSeparator++ );
	*pszCursor = L'\0';
	return szFilename;
}


bool PeModuleDependentOn( const PeImage& TestApp, LPCSTR pszTestModule )
{
	return (PeModuleImplicitDependentOn( TestApp, pszTestModule ) ||
					PeModuleDelayDependentOn( TestApp, pszTestModule ));
}



bool PeModuleImplicitDependentOn( const PeImage& TestApp, LPCSTR pszTestModule )
{
	ULONG cbImportTable;
	PIMAGE_IMPORT_DESCRIPTOR pImportTable = TestApp.GetImportTable( cbImportTable );
	if( pImportTable == NULL )
		return false;
	MemProtectOverride memRead( pImportTable, cbImportTable, PAGE_READONLY );
	for( ; pImportTable->Name; pImportTable++ )
	{
		const char* pszDependentModule = (const char*)TestApp.TranslateRva( (RVA)pImportTable->Name );
		if( !pszDependentModule )
			continue;
		if( lstrcmpiA( pszDependentModule, pszTestModule ) == 0 )
			return true;
	}
	return false;
}


bool PeModuleDelayDependentOn( const PeImage& TestApp, LPCSTR pszTestModule )
{
	PeDelayLoadImportTable ImportTable( TestApp );
	PPeImgDelayDescr pImportTable = ImportTable;
	if( pImportTable == NULL )
		return NULL; //Exit if there is no import directory

	for( ; pImportTable->rvaDLLName; pImportTable++ )
	{
		bool bRVA = (pImportTable->grAttrs & Pe_dlattrRva);
		const char* pszDependentModule =
			bRVA? (const char*)TestApp.TranslateRva( pImportTable->rvaDLLName ) : (const char*)(DWORD_PTR)pImportTable->rvaDLLName;
		if( !pszDependentModule )
			continue;
		if( lstrcmpiA( pszDependentModule, pszTestModule ) == 0 )
			return true;
	}
	return false;
}


FARPROC* PeGetImportTableEntry( const PeImage& ImportingModule,
																LPCSTR pszExportingModule,
																LPCSTR pszFunction )
{
	FARPROC* pProc = PeGetImplicitImportTableEntry( ImportingModule, pszExportingModule, pszFunction );
	if( pProc )
		return pProc;
	return PeGetDelayLoadImportTableEntry( ImportingModule, pszExportingModule, pszFunction );
}


FARPROC* PeGetImplicitImportTableEntry( const PeImage& ImportingModule,
																				LPCSTR pszExportingModule,
																				LPCSTR pszFunction )
{
	PeImportTable ImportTable( ImportingModule );
	PIMAGE_IMPORT_DESCRIPTOR pImportTable = ImportTable;
	if( pImportTable == NULL )
		return NULL; //Exit if there is no import directory

	for( ; pImportTable->Name; pImportTable++ )
	{
		const char* pszModule = (const char*)ImportingModule.TranslateRva( (RVA)pImportTable->Name );
		if( !pszModule )
			continue;

		if( lstrcmpiA( pszModule, pszExportingModule ) != 0 )
			continue;

		HMODULE hmodSource = ::GetModuleHandleA( pszModule );
		if( !hmodSource ) //It should always be loaded, but check anyway
			continue;

		// Iterate through all imported functions for the source module
		PIMAGE_THUNK_DATA pThunk;
		PIMAGE_THUNK_DATA pBoundThunk;
		PeImage* pUnboundImage = NULL;
		pBoundThunk = (PIMAGE_THUNK_DATA)ImportingModule.TranslateRva( (RVA)pImportTable->FirstThunk );
		if( ImportingModule.IsMappedFile() ||
				pImportTable->FirstThunk != pImportTable->OriginalFirstThunk )
			pThunk = (PIMAGE_THUNK_DATA)ImportingModule.TranslateRva( (RVA)pImportTable->OriginalFirstThunk );
		else // OriginalFirstThunk is the same table as FirstThunk, so use an unbound version
		{
			pUnboundImage = new PeImage( PeGetModulePath( ImportingModule ) );
			if( !pUnboundImage )
				return NULL;
			PIMAGE_IMPORT_DESCRIPTOR pUnboundImportTable = pUnboundImage->GetImportTable();
			if( pUnboundImportTable == NULL )
			{
				delete pUnboundImage;
				return NULL; //Something isn't right!
			}
			pThunk =
				(PIMAGE_THUNK_DATA)pUnboundImage->TranslateRva( (RVA)pUnboundImportTable->OriginalFirstThunk );
		}
		for ( ; pThunk->u1.Function; pThunk++, pBoundThunk++ )
		{
			if( IMAGE_SNAP_BY_ORDINAL(pThunk->u1.Ordinal) ) //If imported by ordinal
			{
				if( 0 == ((DWORD)(DWORD_PTR)pszFunction & 0xFFFF0000) &&
						IMAGE_ORDINAL(pThunk->u1.Ordinal) == IMAGE_ORDINAL((DWORD)(DWORD_PTR)pszFunction) )
				{
					if( pUnboundImage )
						delete pUnboundImage;
					return (FARPROC*)&pBoundThunk->u1.Function;
				}
			}
			else
			{
				if( inline_strcmp( (const char*)((PIMAGE_IMPORT_BY_NAME)ImportingModule.TranslateRva( (RVA)pThunk->u1.AddressOfData ))->Name, pszFunction ) == 0 )
				{
					if( pUnboundImage )
						delete pUnboundImage;
					return (FARPROC*)&pBoundThunk->u1.Function;
				}
			}
		}
		if( pUnboundImage )
			delete pUnboundImage;
	}

	return NULL;
}


FARPROC* PeGetDelayLoadImportTableEntry( const PeImage& ImportingModule,
																				 LPCSTR pszExportingModule,
																				 LPCSTR pszFunction )
{
	PeDelayLoadImportTable ImportTable( ImportingModule );
	PPeImgDelayDescr pImportTable = ImportTable;
	if( pImportTable == NULL )
		return NULL; //Exit if there is no import directory

	for( ; pImportTable->rvaDLLName; pImportTable++ )
	{
		bool bRVA = (pImportTable->grAttrs & Pe_dlattrRva);
		const char* pszModule =
			bRVA? (const char*)ImportingModule.TranslateRva( pImportTable->rvaDLLName ) : (const char*)(DWORD_PTR)pImportTable->rvaDLLName;
		if( !pszModule )
			continue;

		if( lstrcmpiA( pszModule, pszExportingModule ) != 0 )
			continue;

		// Iterate through all imported functions for the source module
		PIMAGE_THUNK_DATA pIAT =
			bRVA?
				(PIMAGE_THUNK_DATA)ImportingModule.TranslateRva( pImportTable->rvaIAT ) :
				(PIMAGE_THUNK_DATA)(DWORD_PTR)pImportTable->rvaIAT;
		PIMAGE_THUNK_DATA pINT =
			bRVA?
				(PIMAGE_THUNK_DATA)ImportingModule.TranslateRva( pImportTable->rvaINT ) :
				(PIMAGE_THUNK_DATA)(DWORD_PTR)pImportTable->rvaINT;
		for ( ; pIAT->u1.Function; pIAT++, pINT++ )
		{
			if( IMAGE_SNAP_BY_ORDINAL(pINT->u1.Ordinal) ) //If imported by ordinal
			{
				if( 0 == ((DWORD)(DWORD_PTR)pszFunction & 0xFFFF0000) &&
						IMAGE_ORDINAL(pINT->u1.Ordinal) == IMAGE_ORDINAL((DWORD)(DWORD_PTR)pszFunction) )
					return (FARPROC*)&pIAT->u1.Function;
			}
			else
			{
				PIMAGE_IMPORT_BY_NAME pHintName =
					bRVA?
						(PIMAGE_IMPORT_BY_NAME)ImportingModule.TranslateRva( (RVA)pINT->u1.AddressOfData ) :
						(PIMAGE_IMPORT_BY_NAME)pINT->u1.AddressOfData;
				if( inline_strcmp( (const char*)pHintName->Name, pszFunction ) == 0 )
					return (FARPROC*)&pIAT->u1.Function;
			}
		}
	}

	return NULL;
}


static LONG InternalSearchExportNameTable( const PeImage& ExportingModule, const RVA* arNames, ULONG ctNames, LPCSTR pszFunction )
{
	//binary search for function name
	DWORD idxStart = 0;
	DWORD idxEnd = ctNames;
	while( idxStart < idxEnd )
	{
		DWORD idxTest = (idxStart + idxEnd) >> 1;
		const char* pName = (const char*)ExportingModule.TranslateRva( arNames[idxTest] );
		int nComp = inline_strcmp( pName, pszFunction ); //must use straight binary comparison!
		if( nComp == 0 )
			return (LONG)idxTest;
		if( nComp < 0 )
			idxStart = idxTest + 1;
		else
			idxEnd = idxTest;
	}
	return -1;
}

LONG PeSearchExportNameTable( const PeImage& ExportingModule, LPCSTR pszFunction )
{
	if( HIWORD(pszFunction) == 0 ) //Ordinal
		return -1;
	PeExportTable ExportTable( ExportingModule );
	PIMAGE_EXPORT_DIRECTORY pExportTable = ExportTable;
	if( pExportTable == NULL ) //Exit if there is no export directory
		return -1;
	DWORD ctNames = pExportTable->NumberOfNames;
	RVA* arNames = (RVA*)ExportingModule.TranslateRva( (RVA)pExportTable->AddressOfNames );
	return InternalSearchExportNameTable( ExportingModule, arNames, ctNames, pszFunction );
}


RVA* PeGetExportTableEntry( const PeImage& ExportingModule, LPCSTR pszFunction )
{
	if( HIWORD(pszFunction) == 0 ) //Ordinal
		return PeGetExportTableEntry( ExportingModule, (ORDINAL)pszFunction );
	PeExportTable ExportTable( ExportingModule );
	PIMAGE_EXPORT_DIRECTORY pExportTable = ExportTable;
	if( pExportTable == NULL ) //Exit if there is no export directory
		return NULL;
	DWORD ctNames = pExportTable->NumberOfNames;
	RVA* arNames = (RVA*)ExportingModule.TranslateRva( (RVA)pExportTable->AddressOfNames );
	LONG idxName = InternalSearchExportNameTable( ExportingModule, arNames, ctNames, pszFunction );
	if( idxName >= 0 )
	{
		RVA* arFunc = (RVA*)ExportingModule.TranslateRva( (RVA)pExportTable->AddressOfFunctions );
		WORD* arNameOrd = (WORD*)ExportingModule.TranslateRva( (RVA)pExportTable->AddressOfNameOrdinals );
		return &arFunc[arNameOrd[idxName]];
	}
	return NULL;
}


RVA* PeGetExportTableEntry( const PeImage& ExportingModule, ORDINAL dwOrdinal )
{
	PeExportTable ExportTable( ExportingModule );
	PIMAGE_EXPORT_DIRECTORY pExportTable = ExportTable;
	if( pExportTable == NULL ) //Exit if there is no export directory
		return NULL;
	DWORD dwOrdinalBase = pExportTable->Base;
	DWORD ctFunc = pExportTable->NumberOfFunctions;
	RVA* arFunc = (RVA*)ExportingModule.TranslateRva( (RVA)pExportTable->AddressOfFunctions );
	if( dwOrdinal < dwOrdinalBase || (dwOrdinal - dwOrdinalBase) >= ctFunc )
		return NULL;
	return &arFunc[dwOrdinal - dwOrdinalBase];
}

FARPROC PeGetProcAddress( const PeImage& ExportingModule, LPCSTR pszFunction )
{
	if( !ExportingModule.IsMappedFile() )
		return GetProcAddress( ExportingModule, pszFunction );
	RVA* prvaExport = PeGetExportTableEntry( ExportingModule, pszFunction );
	if( prvaExport == NULL )
		return NULL;
	return (FARPROC)ExportingModule.TranslateRva( *prvaExport );
}


bool EnumerateBaseRelocations( PeImage& PeSource, ENUMBASERELOCATIONCB pfCallback )
{
	ModuleBaseRelocations PeBaseReloc( PeSource );
	PIMAGE_BASE_RELOCATION pRelocDir = PeBaseReloc.GetRelocationTable();
	PIMAGE_BASE_RELOCATION pEndRelocDir =
		(PIMAGE_BASE_RELOCATION)((DWORD_PTR)pRelocDir + PeBaseReloc.GetRelocationTableSize());
	do
	{
		WORD* pEndReloc = (WORD*)((DWORD_PTR)pRelocDir + pRelocDir->SizeOfBlock);
		WORD* pReloc = (WORD*)((DWORD_PTR)pRelocDir + sizeof(IMAGE_BASE_RELOCATION));
		for( ; pReloc < pEndReloc; pReloc++ )
		{
			if( !pfCallback( PeBaseRelocation( PeSource, pRelocDir, pReloc ) ) )
				return false; //End processing
		}
		pRelocDir = (PIMAGE_BASE_RELOCATION)(((((DWORD_PTR)pEndReloc - 1) >> 2) << 2) + 4);
	} while( pRelocDir < pEndRelocDir );

	return true;
}

bool CompareNameOrdinal( LPCSTR pszFunction1, LPCSTR pszFunction2 )
{
	if( IsOrdinal(pszFunction1) )
	{
		if( IsOrdinal(pszFunction2) )
			return ((ORDINAL)pszFunction1 == (ORDINAL)pszFunction2);
		return false;
	}
	if( IsOrdinal(pszFunction2) )
		return false;
	return (inline_strcmp( pszFunction1, pszFunction2 ) == 0);
}
