// Copyright 2010 ManuSoft
// https://www.manusoft.com
// All Rights Reserved
//
// Use of this software is governed by the terms of:
// GNU General Public License v2.0
// See LICENSE for details

#pragma once

#ifndef _PEIMAGE_H_
#define _PEIMAGE_H_


#include "MemIO.h"

#ifndef DLLEXPORT
#define DLLEXPORT
#endif

#if (_MSC_VER < 1400)
#define __unaligned
#endif

#ifdef _WIN64
typedef __unaligned DWORD RVA;
#else
typedef DWORD RVA;
#define DWORD_PTR DWORD
#endif

#ifdef IMAGE_NT_OPTIONAL_HDR64_MAGIC
#define PE32PLUS 1
#ifndef IMAGE_DEBUG_TYPE_CLSID //if this isn't defined, neither is PIMAGE_LOAD_CONFIG_DIRECTORY32
#define PIMAGE_LOAD_CONFIG_DIRECTORY32 PIMAGE_LOAD_CONFIG_DIRECTORY
#define PE32PLUS_NO_LOAD_CONFIG_DIRECTORY64 1
#endif
#else //pre-X64 Windows SDK doesn't define PE32+ structs
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC      0x20b
#define IMAGE_ORDINAL32(Ordinal) (Ordinal & 0xffff)
#define PIMAGE_NT_HEADERS32 PIMAGE_NT_HEADERS
#define PIMAGE_LOAD_CONFIG_DIRECTORY32 PIMAGE_LOAD_CONFIG_DIRECTORY
#define PIMAGE_TLS_DIRECTORY32 PIMAGE_TLS_DIRECTORY
#endif



//************************************************************************************
//Delay load import table descriptor copied from DelayHlp.h (VC7 version)
typedef struct /*ImgDelayDescr*/ {
		DWORD           grAttrs;        // attributes
		RVA             rvaDLLName;     // RVA to dll name
		RVA             rvaHmod;        // RVA of module handle
		RVA             rvaIAT;         // RVA of the IAT
		RVA             rvaINT;         // RVA of the INT
		RVA             rvaBoundIAT;    // RVA of the optional bound IAT
		RVA             rvaUnloadIAT;   // RVA of optional copy of original IAT
		DWORD           dwTimeStamp;    // 0 if not bound,
																		// O.W. date/time stamp of DLL bound to (Old BIND)
		} PeImgDelayDescr, * PPeImgDelayDescr;
typedef const PeImgDelayDescr *   PCPeImgDelayDescr;
enum /*DLAttr*/ {                   // Delay Load Attributes
		Pe_dlattrRva = 0x1,             // RVAs are used instead of pointers
																		// Having this set indicates a VC7.0
																		// and above delay load descriptor.
		};
#define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT   13   // Delay Load Import Descriptors
//************************************************************************************


class DLLEXPORT PeFile
{
protected:
	HANDLE m_hFile;
	HANDLE m_hMapping;
	LPVOID m_pImage;
	char m_szModulePath[MAX_PATH];
public:
	PeFile( LPCSTR pszModulePath, bool bForWrite = false );
	PeFile( LPCWSTR pszModulePath, bool bForWrite = false );
	~PeFile();
	operator HMODULE() const { return (HMODULE)m_pImage; }
	LPCSTR GetModulePath() const { return m_szModulePath; }
};


class DLLEXPORT PeImage
{
protected:
	HMODULE m_hmodImage;
	PIMAGE_DOS_HEADER m_pDosHeader;
	PIMAGE_NT_HEADERS32 m_pNTHeader;
	PeFile* m_pPeFile;

	void Init();
public:
	PeImage( HMODULE hmodTarget );
	PeImage( LPCSTR pszModulePath, bool bForWrite = false );
	PeImage( LPCWSTR pszModulePath, bool bForWrite = false );
	~PeImage();

	operator HMODULE() const { return m_hmodImage; }
	bool IsMappedFile() const { return (m_pPeFile != NULL); }
	PeFile* GetMappedFile() const { return m_pPeFile; }

	bool IsPEModule() const
		{ return (m_pNTHeader != NULL); }
	bool IsPE32Plus() const
		{ return (IsPEModule() && m_pNTHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC); }
	WORD GetCharacteristics() const { return (m_pNTHeader? m_pNTHeader->FileHeader.Characteristics : (WORD)0); }

	LPVOID TranslateRva( RVA rva ) const;
	RVA TranslateAddress( LPVOID pvAddress ) const;
	DWORD_PTR GetSizeOfImage() const;
	PIMAGE_DATA_DIRECTORY GetDirectoryEntry( USHORT idxDirectory ) const;
	LPVOID GetDirectoryData( USHORT idxDirectory ) const;
	LPVOID GetDirectoryData( USHORT idxDirectory, ULONG& ulSize ) const;
	USHORT GetSectionCount() const;
	PIMAGE_SECTION_HEADER GetSectionHeader( USHORT idxSection ) const;
	PIMAGE_SECTION_HEADER GetSectionHeaderForRva( RVA rva ) const;
	PIMAGE_DOS_HEADER GetDosHeader() const { return m_pDosHeader; }
	PIMAGE_NT_HEADERS32 GetNtHeader32() const { return (IsPE32Plus()? NULL : m_pNTHeader); }
#ifdef PE32PLUS
	PIMAGE_NT_HEADERS64 GetNtHeader64() const { return (IsPE32Plus()? (PIMAGE_NT_HEADERS64)m_pNTHeader : NULL); }
#endif
#ifndef _WIN64
	PIMAGE_NT_HEADERS32 GetNtHeader() const { return GetNtHeader32(); }
#elif PE32PLUS
	PIMAGE_NT_HEADERS64 GetNtHeader() const { return GetNtHeader64(); }
#endif

	DWORD_PTR GetActualBaseAddress() const { return (DWORD_PTR)m_pDosHeader; }
	DWORD_PTR GetPreferredBaseAddress() const;

	//Inline functions to access directories
	PIMAGE_IMPORT_DESCRIPTOR GetImportTable() const;
	PIMAGE_IMPORT_DESCRIPTOR GetImportTable( ULONG& ulSize ) const;
	PPeImgDelayDescr GetDelayLoadTable() const;
	PPeImgDelayDescr GetDelayLoadTable( ULONG& ulSize ) const;
	PIMAGE_EXPORT_DIRECTORY GetExportTable() const;
	PIMAGE_EXPORT_DIRECTORY GetExportTable( ULONG& ulSize ) const;
	PIMAGE_TLS_DIRECTORY32 GetTLSDirectory32() const;
	PIMAGE_TLS_DIRECTORY32 GetTLSDirectory32( ULONG& ulSize ) const;
#ifdef PE32PLUS
	PIMAGE_TLS_DIRECTORY64 GetTLSDirectory64() const;
	PIMAGE_TLS_DIRECTORY64 GetTLSDirectory64( ULONG& ulSize ) const;
#endif
#ifdef _WIN64
	PIMAGE_TLS_DIRECTORY64 GetTLSDirectory() const { return GetTLSDirectory64(); }
	PIMAGE_TLS_DIRECTORY64 GetTLSDirectory( ULONG& ulSize ) const { return GetTLSDirectory64( ulSize ); }
#elif PE32PLUS
	PIMAGE_TLS_DIRECTORY32 GetTLSDirectory() const { return GetTLSDirectory32(); }
	PIMAGE_TLS_DIRECTORY32 GetTLSDirectory( ULONG& ulSize ) const { return GetTLSDirectory32( ulSize ); }
#endif
	PIMAGE_BOUND_IMPORT_DESCRIPTOR GetBoundImportTable() const;
	PIMAGE_BOUND_IMPORT_DESCRIPTOR GetBoundImportTable( ULONG& ulSize ) const;
	PIMAGE_RESOURCE_DIRECTORY GetResourceDirectory() const;
	PIMAGE_RESOURCE_DIRECTORY GetResourceDirectory( ULONG& ulSize ) const;
	PIMAGE_LOAD_CONFIG_DIRECTORY32 GetLoadConfigurationDirectory32() const;
	PIMAGE_LOAD_CONFIG_DIRECTORY32 GetLoadConfigurationDirectory32( ULONG& ulSize ) const;
#ifdef PE32PLUS
#ifndef PE32PLUS_NO_LOAD_CONFIG_DIRECTORY64
	PIMAGE_LOAD_CONFIG_DIRECTORY64 GetLoadConfigurationDirectory64() const;
	PIMAGE_LOAD_CONFIG_DIRECTORY64 GetLoadConfigurationDirectory64( ULONG& ulSize ) const;
#endif
#endif
#ifdef _WIN64
#ifndef PE32PLUS_NO_LOAD_CONFIG_DIRECTORY64
	PIMAGE_LOAD_CONFIG_DIRECTORY64 GetLoadConfigurationDirectory() const { return GetLoadConfigurationDirectory64(); }
	PIMAGE_LOAD_CONFIG_DIRECTORY64 GetLoadConfigurationDirectory( ULONG& ulSize ) const { return GetLoadConfigurationDirectory64( ulSize ); }
#endif
#elif PE32PLUS
	PIMAGE_LOAD_CONFIG_DIRECTORY32 GetLoadConfigurationDirectory() const { return GetLoadConfigurationDirectory32(); }
	PIMAGE_LOAD_CONFIG_DIRECTORY32 GetLoadConfigurationDirectory( ULONG& ulSize ) const { return GetLoadConfigurationDirectory32( ulSize ); }
#endif
	PIMAGE_DEBUG_DIRECTORY GetDebugDirectory() const;
	PIMAGE_DEBUG_DIRECTORY GetDebugDirectory( ULONG& ulSize ) const;
	PIMAGE_BASE_RELOCATION GetBaseRelocationDirectory() const;
	PIMAGE_BASE_RELOCATION GetBaseRelocationDirectory( ULONG& ulSize ) const;
};


template<class PeDir,USHORT idx>
class PeTable
{
	const PeImage* m_pPeImage;
	bool m_bMustDelete;
	PeDir m_pTable;
	MemProtectOverride* m_pMemProtect;
public:
	PeTable( const PeImage& peImage, bool bForWrite = false );
	PeTable( HMODULE hmodTarget, bool bForWrite = false );
	PeTable( LPCSTR pszModulePath, bool bForWrite = false );
	PeTable( LPCWSTR pszModulePath, bool bForWrite = false );
	~PeTable();
	LPVOID TranslateRva( RVA dwRva ) const;
	bool IsPE32Plus() const;
	operator PeDir() const { return m_pTable; }
};

typedef PeTable<PIMAGE_IMPORT_DESCRIPTOR,IMAGE_DIRECTORY_ENTRY_IMPORT> PeImportTable;
typedef PeTable<PPeImgDelayDescr,IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT> PeDelayLoadImportTable;
typedef PeTable<PIMAGE_EXPORT_DIRECTORY,IMAGE_DIRECTORY_ENTRY_EXPORT> PeExportTable;

template<class PeDir,USHORT idx>
inline PeTable<PeDir,idx>::PeTable( const PeImage& peImage, bool bForWrite /*= false*/ )
:	m_pPeImage( &peImage ),
	m_bMustDelete( FALSE ),
	m_pMemProtect( NULL )
{
	ULONG cbTable;
	m_pTable = (PeDir)m_pPeImage->GetDirectoryData( idx, cbTable );
	if( m_pTable )
	{
		m_pMemProtect =
			new MemProtectOverride( m_pTable, cbTable,
															bForWrite? PAGE_READWRITE : PAGE_READONLY );
	}
}

template<class PeDir,USHORT idx>
inline PeTable<PeDir,idx>::PeTable( HMODULE hmodTarget, bool bForWrite /*= false*/ )
:	m_pPeImage( new PeImage( hmodTarget ) ),
	m_bMustDelete( true ),
	m_pMemProtect( NULL )
{
	ULONG cbTable;
	m_pTable = (PeDir)m_pPeImage->GetDirectoryData( idx, cbTable );
	if( m_pTable )
	{
		m_pMemProtect =
			new MemProtectOverride( m_pTable, cbTable,
															bForWrite? PAGE_READWRITE : PAGE_READONLY );
	}
}

template<class PeDir,USHORT idx>
inline PeTable<PeDir,idx>::PeTable( LPCSTR pszModulePath, bool bForWrite /*= false*/ )
:	m_pPeImage( new PeImage( pszModulePath, bForWrite ) ),
	m_bMustDelete( true ),
	m_pMemProtect( NULL )
{
	m_pTable = (PeDir)m_pPeImage->GetDirectoryData( idx );
}

template<class PeDir,USHORT idx>
inline PeTable<PeDir,idx>::PeTable( LPCWSTR pszModulePath, bool bForWrite /*= false*/ )
:	m_pPeImage( new PeImage( pszModulePath, bForWrite ) ),
	m_bMustDelete( true ),
	m_pMemProtect( NULL )
{
	m_pTable = (PeDir)m_pPeImage->GetDirectoryData( idx );
}

template<class PeDir,USHORT idx>
inline PeTable<PeDir,idx>::~PeTable()
{
	delete m_pMemProtect;
	if( m_bMustDelete )
		delete m_pPeImage;
}

template<class PeDir,USHORT idx>
inline LPVOID PeTable<PeDir,idx>::TranslateRva( RVA dwRva ) const
{
	return m_pPeImage->TranslateRva( dwRva );
}

template<class PeDir,USHORT idx>
inline bool PeTable<PeDir,idx>::IsPE32Plus() const
{
	return m_pPeImage->IsPE32Plus();
}


class PeSection
{
	const PeImage* m_pPeImage;
	bool m_bMustDelete;
	BYTE* m_pSection;
	ULONG m_cbSection;
	MemProtectOverride* m_pMemProtect;
public:
	PeSection( const PeImage& peImage, USHORT idxSection, bool bForWrite = false );
	PeSection( HMODULE hmodTarget, USHORT idxSection, bool bForWrite = false );
	PeSection( LPCSTR pszModulePath, USHORT idxSection, bool bForWrite = false );
	PeSection( LPCWSTR pszModulePath, USHORT idxSection, bool bForWrite = false );
	~PeSection();
	LPVOID TranslateRva( RVA dwRva ) const;
	operator BYTE*() const { return m_pSection; }
	ULONG GetSize() const { return m_cbSection; }
};

inline PeSection::~PeSection()
{
	delete m_pMemProtect;
	if( m_bMustDelete )
		delete m_pPeImage;
}

inline LPVOID PeSection::TranslateRva( RVA dwRva ) const
{
	return m_pPeImage->TranslateRva( dwRva );
}


inline
PIMAGE_IMPORT_DESCRIPTOR PeImage::GetImportTable() const
{
	return (PIMAGE_IMPORT_DESCRIPTOR)GetDirectoryData( IMAGE_DIRECTORY_ENTRY_IMPORT );
}

inline
PIMAGE_IMPORT_DESCRIPTOR PeImage::GetImportTable( ULONG& ulSize ) const
{
	return (PIMAGE_IMPORT_DESCRIPTOR)GetDirectoryData( IMAGE_DIRECTORY_ENTRY_IMPORT, ulSize );
}


inline
PPeImgDelayDescr PeImage::GetDelayLoadTable() const
{
	return (PPeImgDelayDescr)GetDirectoryData( IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT );
}

inline
PPeImgDelayDescr PeImage::GetDelayLoadTable( ULONG& ulSize ) const
{
	return (PPeImgDelayDescr)GetDirectoryData( IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT, ulSize );
}

inline
PIMAGE_EXPORT_DIRECTORY PeImage::GetExportTable() const
{
	return (PIMAGE_EXPORT_DIRECTORY)GetDirectoryData( IMAGE_DIRECTORY_ENTRY_EXPORT );
}

inline
PIMAGE_EXPORT_DIRECTORY PeImage::GetExportTable( ULONG& ulSize ) const
{
	return (PIMAGE_EXPORT_DIRECTORY)GetDirectoryData( IMAGE_DIRECTORY_ENTRY_EXPORT, ulSize );
}

inline
PIMAGE_TLS_DIRECTORY32 PeImage::GetTLSDirectory32() const
{
	return (PIMAGE_TLS_DIRECTORY32)GetDirectoryData( IMAGE_DIRECTORY_ENTRY_TLS );
}

inline
PIMAGE_TLS_DIRECTORY32 PeImage::GetTLSDirectory32( ULONG& ulSize ) const
{
	return (PIMAGE_TLS_DIRECTORY32)GetDirectoryData( IMAGE_DIRECTORY_ENTRY_TLS, ulSize );
}

#ifdef PE32PLUS

inline
PIMAGE_TLS_DIRECTORY64 PeImage::GetTLSDirectory64() const
{
	return (PIMAGE_TLS_DIRECTORY64)GetDirectoryData( IMAGE_DIRECTORY_ENTRY_TLS );
}

inline
PIMAGE_TLS_DIRECTORY64 PeImage::GetTLSDirectory64( ULONG& ulSize ) const
{
	return (PIMAGE_TLS_DIRECTORY64)GetDirectoryData( IMAGE_DIRECTORY_ENTRY_TLS, ulSize );
}

#endif //PE32PLUS

inline
PIMAGE_BOUND_IMPORT_DESCRIPTOR PeImage::GetBoundImportTable() const
{
	return (PIMAGE_BOUND_IMPORT_DESCRIPTOR)GetDirectoryData( IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT );
}

inline
PIMAGE_BOUND_IMPORT_DESCRIPTOR PeImage::GetBoundImportTable( ULONG& ulSize ) const
{
	return (PIMAGE_BOUND_IMPORT_DESCRIPTOR)GetDirectoryData( IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT, ulSize );
}

inline
PIMAGE_RESOURCE_DIRECTORY PeImage::GetResourceDirectory() const
{
	return (PIMAGE_RESOURCE_DIRECTORY)GetDirectoryData( IMAGE_DIRECTORY_ENTRY_RESOURCE );
}

inline
PIMAGE_RESOURCE_DIRECTORY PeImage::GetResourceDirectory( ULONG& ulSize ) const
{
	return (PIMAGE_RESOURCE_DIRECTORY)GetDirectoryData( IMAGE_DIRECTORY_ENTRY_RESOURCE, ulSize );
}

inline
PIMAGE_LOAD_CONFIG_DIRECTORY32 PeImage::GetLoadConfigurationDirectory32() const
{
	if( IsPE32Plus() )
		return NULL;
	return (PIMAGE_LOAD_CONFIG_DIRECTORY32)GetDirectoryData( IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG );
}

inline
PIMAGE_LOAD_CONFIG_DIRECTORY32 PeImage::GetLoadConfigurationDirectory32( ULONG& ulSize ) const
{
	if( IsPE32Plus() )
		return NULL;
	return (PIMAGE_LOAD_CONFIG_DIRECTORY32)GetDirectoryData( IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG, ulSize );
}

#ifdef PE32PLUS
#ifndef PE32PLUS_NO_LOAD_CONFIG_DIRECTORY64

inline
PIMAGE_LOAD_CONFIG_DIRECTORY64 PeImage::GetLoadConfigurationDirectory64() const
{
	if( !IsPE32Plus() )
		return NULL;
	return (PIMAGE_LOAD_CONFIG_DIRECTORY64)GetDirectoryData( IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG );
}

inline
PIMAGE_LOAD_CONFIG_DIRECTORY64 PeImage::GetLoadConfigurationDirectory64( ULONG& ulSize ) const
{
	if( !IsPE32Plus() )
		return NULL;
	return (PIMAGE_LOAD_CONFIG_DIRECTORY64)GetDirectoryData( IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG, ulSize );
}

#endif //PE32PLUS_NO_LOAD_CONFIG_DIRECTORY64
#endif //PE32PLUS

inline
PIMAGE_DEBUG_DIRECTORY PeImage::GetDebugDirectory() const
{
	return (PIMAGE_DEBUG_DIRECTORY)GetDirectoryData( IMAGE_DIRECTORY_ENTRY_DEBUG );
}

inline
PIMAGE_DEBUG_DIRECTORY PeImage::GetDebugDirectory( ULONG& ulSize ) const
{
	return (PIMAGE_DEBUG_DIRECTORY)GetDirectoryData( IMAGE_DIRECTORY_ENTRY_DEBUG, ulSize );
}

inline
PIMAGE_BASE_RELOCATION PeImage::GetBaseRelocationDirectory() const
{
	return (PIMAGE_BASE_RELOCATION)GetDirectoryData( IMAGE_DIRECTORY_ENTRY_BASERELOC );
}

inline
PIMAGE_BASE_RELOCATION PeImage::GetBaseRelocationDirectory( ULONG& ulSize ) const
{
	return (PIMAGE_BASE_RELOCATION)GetDirectoryData( IMAGE_DIRECTORY_ENTRY_BASERELOC, ulSize );
}


#endif //_PEIMAGE_H_
