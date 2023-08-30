// Copyright 2010 ManuSoft
// https://www.manusoft.com
// All Rights Reserved
//
// Use of this software is governed by the terms of:
// GNU General Public License v2.0
// See LICENSE for details

#include "StdAfx.h"
#include "PeImage.h"
#include "PeUtility.h"


// Class member implementations
PeFile::PeFile( LPCSTR pszModulePath, bool bForWrite /*= false*/ )
:	m_hFile( CreateFileA( pszModulePath,
												bForWrite? GENERIC_READ | GENERIC_WRITE : GENERIC_READ,
												FILE_SHARE_READ,
												NULL,
												OPEN_EXISTING,
												0,
												NULL ) ),
	m_hMapping( NULL ),
	m_pImage( NULL )
{
	lstrcpynA( m_szModulePath, pszModulePath, MAX_PATH );
	m_hMapping = CreateFileMapping( m_hFile,
																	NULL,
																	bForWrite? PAGE_READWRITE : PAGE_READONLY,
																	0,
																	0,
																	NULL );
	CloseHandle( m_hFile );
	if( m_hMapping )
	{
		m_pImage = MapViewOfFile( m_hMapping,
															bForWrite? FILE_MAP_ALL_ACCESS : FILE_MAP_READ,
															0,
															0,
															0 );
	}
}


PeFile::PeFile( LPCWSTR pszModulePath, bool bForWrite /*= false*/ )
:	m_hFile( CreateFileW( pszModulePath,
												bForWrite? GENERIC_READ | GENERIC_WRITE : GENERIC_READ,
												FILE_SHARE_READ,
												NULL,
												OPEN_EXISTING,
												0,
												NULL ) ),
	m_hMapping( NULL ),
	m_pImage( NULL )
{
	m_szModulePath[0] = '\0';
	WideCharToMultiByte( CP_ACP,
											 0,
											 pszModulePath,
											 -1,
											 m_szModulePath,
											 sizeof(m_szModulePath) / sizeof(m_szModulePath[0]),
											 NULL,
											 NULL );
	m_hMapping = CreateFileMapping( m_hFile,
																	NULL,
																	bForWrite? PAGE_READWRITE : PAGE_READONLY,
																	0,
																	0,
																	NULL );
	CloseHandle( m_hFile );
	if( m_hMapping )
	{
		m_pImage = MapViewOfFile( m_hMapping,
															bForWrite? FILE_MAP_ALL_ACCESS : FILE_MAP_READ,
															0,
															0,
															0 );
	}
}


PeFile::~PeFile()
{
	if( m_pImage )
		UnmapViewOfFile( m_pImage );
	if( m_hMapping )
		CloseHandle( m_hMapping );
}


PeImage::PeImage( HMODULE hmodTarget )
:	m_hmodImage( hmodTarget ),
	m_pDosHeader( PeGetModuleBase( hmodTarget ) ),
	m_pNTHeader( NULL ),
	m_pPeFile( NULL )
{
	if( m_hmodImage )
		LoadLibrary( PeGetModulePath( m_hmodImage ) );
	Init();
}


PeImage::PeImage( LPCSTR pszModulePath, bool bForWrite /*= false*/ )
:	m_hmodImage( NULL ),
	m_pDosHeader( NULL ),
	m_pNTHeader( NULL ),
	m_pPeFile( new PeFile( pszModulePath, bForWrite ) )
{
	if( m_pPeFile )
	{
		m_hmodImage = (HMODULE)*m_pPeFile;
		m_pDosHeader = PeGetModuleBase( m_hmodImage );
		Init();
	}
}


PeImage::PeImage( LPCWSTR pszModulePath, bool bForWrite /*= false*/ )
:	m_hmodImage( NULL ),
	m_pDosHeader( NULL ),
	m_pNTHeader( NULL ),
	m_pPeFile( new PeFile( pszModulePath, bForWrite ) )
{
	if( m_pPeFile )
	{
		m_hmodImage = (HMODULE)*m_pPeFile;
		m_pDosHeader = PeGetModuleBase( m_hmodImage );
		Init();
	}
}


void PeImage::Init()
{
	// Tests to make sure we're looking at a module image (the 'MZ' header)
	if( !IsBadReadPtr( m_pDosHeader, sizeof(IMAGE_DOS_HEADER) ) &&
			m_pDosHeader->e_magic == IMAGE_DOS_SIGNATURE )
	{
		m_pNTHeader = (PIMAGE_NT_HEADERS32)((BYTE*)m_pDosHeader + m_pDosHeader->e_lfanew);
		// More tests to make sure we're looking at a "PE" image
		if( IsBadReadPtr( m_pNTHeader, (IsPE32Plus()? sizeof(IMAGE_NT_HEADERS64) : sizeof(IMAGE_NT_HEADERS32)) ) ||
				m_pNTHeader->Signature != IMAGE_NT_SIGNATURE )
			m_pNTHeader = NULL;
	}
}


PeImage::~PeImage()
{
	if( m_pPeFile )
		delete m_pPeFile;
	else if( m_hmodImage )
		FreeLibrary( m_hmodImage );
}


LPVOID PeImage::TranslateRva( RVA rva ) const
{
	USHORT idxSection = GetSectionCount();
	while( idxSection > 0 )
	{
		PIMAGE_SECTION_HEADER pCursor = GetSectionHeader( --idxSection );
		RVA rvaStart = pCursor->VirtualAddress;
		if( rva >= rvaStart &&
				rva < rvaStart + pCursor->Misc.VirtualSize )
		{
			BYTE* rvaBased = (BYTE*)m_pDosHeader + rva;
			if( m_pPeFile )
				return rvaBased + pCursor->PointerToRawData - pCursor->VirtualAddress;
			return rvaBased;
		}
	}
	return NULL;
}


RVA PeImage::TranslateAddress( LPVOID pvAddress ) const
{
#ifdef _WIN64
	ULONGLONG rva64 = (BYTE*)pvAddress - (BYTE*)m_pDosHeader;
	if( rva64 != (RVA)rva64 )
		return (RVA)-1;
#endif //_WIN64
	RVA rvaBased = RVA((BYTE*)pvAddress - (BYTE*)m_pDosHeader);
	USHORT idxSection = GetSectionCount();
	while( idxSection > 0 )
	{
		PIMAGE_SECTION_HEADER pCursor = GetSectionHeader( --idxSection );
		RVA rvaAddress = rvaBased;
		if( m_pPeFile )
			rvaAddress = (rvaAddress + pCursor->VirtualAddress - pCursor->PointerToRawData);
		RVA rvaStart = pCursor->VirtualAddress;
		RVA rvaEnd = rvaStart + (m_pPeFile? pCursor->SizeOfRawData : pCursor->Misc.VirtualSize);
		if( rvaAddress >= rvaStart && rvaAddress < rvaEnd )
			return rvaAddress;
	}
	return ~(RVA)0;
}


DWORD_PTR PeImage::GetSizeOfImage() const
{
	if( !m_pNTHeader )
		return 0;
	PIMAGE_SECTION_HEADER pLastSection = GetSectionHeader( (USHORT)(GetSectionCount() - 1) );
	if( m_pPeFile )
		return (DWORD_PTR)pLastSection->VirtualAddress + pLastSection->SizeOfRawData;
	else
		return (DWORD_PTR)pLastSection->VirtualAddress + pLastSection->Misc.VirtualSize;
}


PIMAGE_DATA_DIRECTORY PeImage::GetDirectoryEntry( USHORT idxDirectory ) const
{
	if( !IsPEModule() )
		return NULL;
	DWORD ctDirectory = 0;
	if( IsPE32Plus() )
	{
	#ifndef PE32PLUS
		return NULL;
	#else
		PIMAGE_NT_HEADERS64 pNtHeader64 = GetNtHeader64();
		if( !pNtHeader64 )
			return NULL;
		ctDirectory = pNtHeader64->OptionalHeader.NumberOfRvaAndSizes;
		if( idxDirectory >= ctDirectory )
			return NULL;
		return (PIMAGE_DATA_DIRECTORY)&(pNtHeader64->OptionalHeader.DataDirectory[idxDirectory]);
	#endif
	}
	PIMAGE_NT_HEADERS32 pNtHeader32 = GetNtHeader32();
	if( !pNtHeader32 )
		return NULL;
	ctDirectory = pNtHeader32->OptionalHeader.NumberOfRvaAndSizes;
	if( idxDirectory >= ctDirectory )
		return NULL;
	return (PIMAGE_DATA_DIRECTORY)&(pNtHeader32->OptionalHeader.DataDirectory[idxDirectory]);
}


LPVOID PeImage::GetDirectoryData( USHORT idxDirectory ) const
{
	ULONG ulDiscard;
	return GetDirectoryData( idxDirectory, ulDiscard );
}


LPVOID PeImage::GetDirectoryData( USHORT idxDirectory, ULONG& ulSize ) const
{
	PIMAGE_DATA_DIRECTORY pDirectory = GetDirectoryEntry( idxDirectory );
	if( !pDirectory )
		return NULL;
	LPVOID pvReturn = TranslateRva( pDirectory->VirtualAddress );
	if( pvReturn )
		ulSize = pDirectory->Size;
	return pvReturn;
}


USHORT PeImage::GetSectionCount() const
{
	if( !m_pNTHeader )
		return 0;
	if( IsPE32Plus() )
	{
	#ifndef PE32PLUS
		return 0;
	#else
		PIMAGE_NT_HEADERS64 pNtHeader64 = GetNtHeader64();
		if( !pNtHeader64 )
			return 0;
		return pNtHeader64->FileHeader.NumberOfSections;
	#endif
	}
	PIMAGE_NT_HEADERS32 pNtHeader32 = GetNtHeader32();
	if( !pNtHeader32 )
		return 0;
	return pNtHeader32->FileHeader.NumberOfSections;
}


PIMAGE_SECTION_HEADER PeImage::GetSectionHeader( USHORT idxSection ) const
{
	if( idxSection >= GetSectionCount() )
		return NULL;
	PIMAGE_SECTION_HEADER pFirstSection = NULL;
	if( IsPE32Plus() )
	{
	#ifndef PE32PLUS
		return NULL;
	#else
		PIMAGE_NT_HEADERS64 pNtHeader64 = GetNtHeader64();
		if( !pNtHeader64 )
			return NULL;
		pFirstSection = (PIMAGE_SECTION_HEADER)((BYTE*)&(pNtHeader64->OptionalHeader) + pNtHeader64->FileHeader.SizeOfOptionalHeader);
	#endif
	}
	else
	{
		PIMAGE_NT_HEADERS32 pNtHeader32 = GetNtHeader32();
		if( !pNtHeader32 )
			return NULL;
		pFirstSection = (PIMAGE_SECTION_HEADER)((BYTE*)&(pNtHeader32->OptionalHeader) + pNtHeader32->FileHeader.SizeOfOptionalHeader);
	}
	return pFirstSection + idxSection;
}


PIMAGE_SECTION_HEADER PeImage::GetSectionHeaderForRva( RVA rva ) const
{
	USHORT idxSection = GetSectionCount();
	while( idxSection > 0 )
	{
		PIMAGE_SECTION_HEADER pCursor = GetSectionHeader( --idxSection );
		RVA rvaStart = pCursor->VirtualAddress;
		RVA rvaEnd = rvaStart + (m_pPeFile? pCursor->SizeOfRawData : pCursor->Misc.VirtualSize);
		if( rva >= rvaStart && rva < rvaEnd )
			return pCursor;
	}
	return NULL;
}


DWORD_PTR PeImage::GetPreferredBaseAddress() const
{
	if( IsPE32Plus() )
	{
	#ifndef _WIN64
		return NULL;
	#else
		PIMAGE_NT_HEADERS64 pNtHeader64 = GetNtHeader64();
		if( !pNtHeader64 )
			return NULL;
		return pNtHeader64->OptionalHeader.ImageBase;
	#endif
	}
	PIMAGE_NT_HEADERS32 pNtHeader32 = GetNtHeader32();
	if( !pNtHeader32 )
		return NULL;
	return pNtHeader32->OptionalHeader.ImageBase;
}


PeSection::PeSection( const PeImage& peImage, USHORT idxSection, bool bForWrite /*= false*/ )
:	m_pPeImage( &peImage ),
	m_bMustDelete( false ),
	m_pSection( NULL ),
	m_cbSection( 0 ),
	m_pMemProtect( NULL )
{
	PIMAGE_SECTION_HEADER pSectionHeader = m_pPeImage->GetSectionHeader( idxSection );
	if( pSectionHeader )
	{
		m_pSection = (BYTE*)TranslateRva( pSectionHeader->VirtualAddress );
		if( m_pSection )
		{
			m_cbSection =
				m_pPeImage->IsMappedFile()?
					pSectionHeader->SizeOfRawData :
					pSectionHeader->Misc.VirtualSize;
			m_pMemProtect =
				new MemProtectOverride( m_pSection,
																m_cbSection,
																bForWrite? PAGE_READWRITE : PAGE_READONLY );
		}
	}
}


PeSection::PeSection( HMODULE hmodTarget, USHORT idxSection, bool bForWrite /*= false*/ )
:	m_pPeImage( new PeImage( hmodTarget ) ),
	m_bMustDelete( true ),
	m_pSection( NULL ),
	m_cbSection( 0 ),
	m_pMemProtect( NULL )
{
	PIMAGE_SECTION_HEADER pSectionHeader = m_pPeImage->GetSectionHeader( idxSection );
	if( pSectionHeader )
	{
		m_pSection = (BYTE*)TranslateRva( pSectionHeader->VirtualAddress );
		if( m_pSection )
		{
			m_cbSection = pSectionHeader->Misc.VirtualSize;
			m_pMemProtect =
				new MemProtectOverride( m_pSection,
																m_cbSection,
																bForWrite? PAGE_READWRITE : PAGE_READONLY );
		}
	}
}


PeSection::PeSection( LPCSTR pszModulePath, USHORT idxSection, bool bForWrite /*= false*/ )
:	m_pPeImage( new PeImage( pszModulePath, bForWrite ) ),
	m_bMustDelete( true ),
	m_pSection( NULL ),
	m_cbSection( 0 ),
	m_pMemProtect( NULL )
{
	PIMAGE_SECTION_HEADER pSectionHeader = m_pPeImage->GetSectionHeader( idxSection );
	if( pSectionHeader )
	{
		m_pSection = (BYTE*)TranslateRva( pSectionHeader->VirtualAddress );
		if( m_pSection )
			m_cbSection = pSectionHeader->SizeOfRawData;
	}
}


PeSection::PeSection( LPCWSTR pszModulePath, USHORT idxSection, bool bForWrite /*= false*/ )
:	m_pPeImage( new PeImage( pszModulePath, bForWrite ) ),
	m_bMustDelete( true ),
	m_pSection( NULL ),
	m_cbSection( 0 ),
	m_pMemProtect( NULL )
{
	PIMAGE_SECTION_HEADER pSectionHeader = m_pPeImage->GetSectionHeader( idxSection );
	if( pSectionHeader )
	{
		m_pSection = (BYTE*)TranslateRva( pSectionHeader->VirtualAddress );
		if( m_pSection )
			m_cbSection = pSectionHeader->SizeOfRawData;
	}
}
