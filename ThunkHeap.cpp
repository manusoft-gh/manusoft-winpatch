// Copyright 2010 ManuSoft
// https://www.manusoft.com
// All Rights Reserved
//
// Use of this software is governed by the terms of:
// GNU General Public License v2.0
// See LICENSE for details

#include "StdAfx.h"
#include "ThunkHeap.h"
#include "PeUtility.h"
#include <TChar.h>
#include <Assert.h>
#include <New.h>
#include <stdlib.h>



static LPCTSTR UniqueNameFromAddress( LPCVOID pvAddress )
{
	static TCHAR szUniqueName[256] = _T("ManuSoft.WinPatchLib.");
#ifndef _WIN64
#if _MSC_VER < 1400
	_ultot( (ULONG)pvAddress, &szUniqueName[21], 16 );
#else
	_ultot_s( (ULONG)pvAddress, &szUniqueName[21], 200, 16 );
#endif
#else
	_ui64tot_s( (unsigned __int64)pvAddress, &szUniqueName[21], 200, 16 );
#endif
	return szUniqueName;
}


#pragma warning(push)
#pragma warning(disable : 4355)
CThunkHeap::CThunkHeap( LPCVOID pvSource, BYTE* pvEndOfHeap )
: mpvSource( pvSource? (LPCVOID)PeGetModuleFromAddress( pvSource ) : (LPCVOID)GetModuleHandle( NULL ) )
, mpvEndOfHeap( pvEndOfHeap )
, mpvNextFree( (BYTE*)(this + 1) )
{
	AddRef();
}
#pragma warning(pop)

void CThunkHeap::FreeHeap()
{
	VirtualFree( (BYTE*)this, mpvEndOfHeap - (BYTE*)this, MEM_RELEASE );
}

ULONG CThunkHeap::AddRef()
{
	DWORD dwOldProt;
	VirtualProtect( &mctRef, sizeof(mctRef), PAGE_EXECUTE_READWRITE, &dwOldProt );
	ULONG ctRef = ++mctRef;
	VirtualProtect( &mctRef, sizeof(mctRef), dwOldProt, &dwOldProt );
	return (++ctRef);
}

ULONG CThunkHeap::Release()
{
	DWORD dwOldProt;
	VirtualProtect( &mctRef, sizeof(mctRef), PAGE_EXECUTE_READWRITE, &dwOldProt );
	--mctRef;
	VirtualProtect( &mctRef, sizeof(mctRef), dwOldProt, &dwOldProt );
	if( mctRef == 0 )
	{
		if( mpvSource )
			UnregisterClass( UniqueNameFromAddress( mpvSource ), (HMODULE)mpvSource );
		FreeHeap();
	}
	return mctRef;
}

BYTE* CThunkHeap::Alloc( size_t cbSize )
{
	if( !mpvNextFree )
		return NULL;
	if( mpvNextFree + cbSize > mpvEndOfHeap )
		return NULL;
	BYTE* pvFreeMem = mpvNextFree;
	DWORD dwOldProt;
	VirtualProtect( &mpvNextFree, sizeof(mpvNextFree), PAGE_EXECUTE_READWRITE, &dwOldProt );
	mpvNextFree += cbSize;
	VirtualProtect( &mpvNextFree, sizeof(mpvNextFree), dwOldProt, &dwOldProt );
	return pvFreeMem;
}

//static
DWORD_PTR CThunkHeap::AlignToPageSize( DWORD_PTR pvAddress )
{
	return (pvAddress - (pvAddress % GetMemAllocGranularity()));
}

//static
LPVOID CThunkHeap::AlignToPageSize( LPVOID pvAddress )
{
	return (LPVOID)AlignToPageSize( (DWORD_PTR)pvAddress );
}

//static
CThunkHeap* CThunkHeap::GetThunkHeap( LPCVOID pvSource, bool bCreate /*= true*/ )
{
	if( !pvSource )
		pvSource = (LPVOID)GetModuleHandle( NULL );
	else
		pvSource = (LPVOID)PeGetModuleFromAddress( pvSource );
	if( !pvSource )
		return NULL;
	LPCTSTR pszUniqueName = UniqueNameFromAddress( pvSource );
	WNDCLASS wcHeapId = { 0 };
	if( GetClassInfo( (HMODULE)pvSource, pszUniqueName, &wcHeapId ) )
	{
		CThunkHeap* pThunkHeap = (CThunkHeap*)wcHeapId.lpfnWndProc;
		return pThunkHeap;
	}
	if( !bCreate )
		return NULL;
	BYTE* pvHeap = NULL;
	DWORD dwMemAllocGranularity = GetMemAllocGranularity();
	DWORD dwAllocSize = dwMemAllocGranularity;
	PeImage Source( (HMODULE)pvSource );
	bool bIsModule = Source.IsPEModule();
	{
		SYSTEM_INFO si = { 0 };
		GetSystemInfo( &si );
		DWORD dwPageSize = si.dwPageSize;
		while(!pvHeap && (dwAllocSize >>= 1) >= dwPageSize)
		{ //try successively smaller allocation sizes until a suitable block is found
			BYTE* pvHeapStartMin = bIsModule ? ((BYTE*)Source.GetActualBaseAddress() + Source.GetSizeOfImage()) : (BYTE*)pvSource;
			BYTE* pvHeapStartMax = (bIsModule ? (BYTE*)Source.GetActualBaseAddress() : (BYTE*)pvSource) + ((DWORD)-1 >> 1) - dwMemAllocGranularity;
			MEMORY_BASIC_INFORMATION mbi = { 0 };
			for (BYTE* pvHeapStart = (BYTE*)AlignToPageSize(pvHeapStartMin) + dwMemAllocGranularity;
				!pvHeap && pvHeapStart <= pvHeapStartMax;
				pvHeapStart = (BYTE*)mbi.BaseAddress + mbi.RegionSize)
			{
				BYTE* pvAlignedHeapStart = (BYTE*)AlignToPageSize(pvHeapStart);
				if( pvHeapStart > pvAlignedHeapStart )
					pvHeapStart += dwMemAllocGranularity;
				if (VirtualQuery(pvHeapStart, &mbi, sizeof(mbi)) == 0)
					break;
				if (mbi.RegionSize < dwAllocSize)
					continue;

				// ****************************************************************
				// Committing previously reserved memory results in unexpected protection change at runtime with ensuing access
				// violation at executing the thunk, so shelving that attempt and looking only for unreserved free memory.
				//
				//if (!(mbi.State & (MEM_FREE | MEM_RESERVE)))
				//	continue;
				//if (!(mbi.State & MEM_FREE) && (mbi.Type & (MEM_IMAGE | MEM_MAPPED)))
				//	continue;
				//DWORD fAllocationType = MEM_COMMIT;
				//if (!(mbi.State & MEM_RESERVE))
				//	fAllocationType |= MEM_RESERVE;
				// ****************************************************************

				if (!(mbi.State & MEM_FREE))
					continue;
				DWORD fAllocationType = (MEM_COMMIT | MEM_RESERVE);
				pvHeap = (BYTE*)VirtualAlloc(pvHeapStart, dwAllocSize, fAllocationType, PAGE_EXECUTE_READWRITE);
				if( pvHeap && (pvHeap < pvHeapStart) )
					dwAllocSize += DWORD(pvHeapStart - pvHeap);
			}
		}
	}
	assert(pvHeap != NULL);
	if( !pvHeap )
		return NULL;

	CThunkHeap* pThunkHeap = new ( pvHeap ) CThunkHeap( pvSource, pvHeap + dwAllocSize );
	DWORD dwOldProt;
	VirtualProtect( pvHeap, dwAllocSize, PAGE_EXECUTE_READ, &dwOldProt );
	WNDCLASS wcHeapIdReg =
	{
		CS_GLOBALCLASS,
		(WNDPROC)pvHeap,
		0,
		0,
		(HMODULE)pvSource,
		NULL,
		NULL,
		NULL,
		NULL,
		pszUniqueName,
	};
	ATOM atomHeapId = RegisterClass( &wcHeapIdReg );
	atomHeapId; //prevent "initialized but not referenced" warning
	assert( atomHeapId != 0 );
	return pThunkHeap;
}

//static
DWORD CThunkHeap::GetMemAllocGranularity()
{
	static DWORD dwAllocGranularity = 0;
	if( dwAllocGranularity > 0 )
		return dwAllocGranularity;
	SYSTEM_INFO si = { 0 };
	GetSystemInfo( &si );
	dwAllocGranularity = si.dwAllocationGranularity;
	return dwAllocGranularity;
}
