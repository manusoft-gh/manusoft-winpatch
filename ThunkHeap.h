// Copyright 2010 ManuSoft
// https://www.manusoft.com
// All Rights Reserved
//
// Use of this software is governed by the terms of:
// GNU General Public License v2.0
// See LICENSE for details

#pragma once

#ifndef _THUNKHEAP_H_
#define _THUNKHEAP_H_

#include <New.h>
#include "Asm.h"


class CThunkHeap
{
	LPCVOID mpvSource;
	ULONG mctRef;
	BYTE* mpvEndOfHeap;
	BYTE* mpvNextFree;
	BYTE mPadding[0x10]; //reserved for future use

protected:
	//unimplemented (to prevent copy assignment)
	CThunkHeap( const CThunkHeap& rhs );
	CThunkHeap& operator =( const CThunkHeap& rhs );

protected:
	CThunkHeap( LPCVOID pvSource, BYTE* pvEndOfHeap );

public:
	BYTE* Alloc( size_t cbSize );
	template< typename T > T* AllocThunk()
		{
			return (T*)Alloc( sizeof(T) );
		}

	//reference counting
	ULONG AddRef();
	ULONG Release();

	static CThunkHeap* GetThunkHeap( LPCVOID pvSource, bool bCreate = true );

public:
	template< typename T >
	static T* CreateThunk( LPCVOID pvSource, PROC pfThunkTarget )
		{
			T* pvThunk = NULL;
			CThunkHeap* pThunkHeap = GetThunkHeap( pvSource );
			if( !pThunkHeap )
				return NULL;
			pvThunk = pThunkHeap->AllocThunk< T >();
			assert( pvThunk != NULL );
			DWORD dwOldProt;
			VirtualProtect( pvThunk, sizeof(T), PAGE_EXECUTE_READWRITE, &dwOldProt );
			new ( pvThunk ) T( pfThunkTarget );
			VirtualProtect( pvThunk, sizeof(T), dwOldProt, &dwOldProt );
			return pvThunk;
		}

private:
	void FreeHeap();

protected:
	static DWORD_PTR AlignToPageSize( DWORD_PTR pvAddress );
	static LPVOID AlignToPageSize( LPVOID pvAddress );
	static DWORD GetMemAllocGranularity();
};

#endif //_THUNKHEAP_H_
