// Copyright 2010 ManuSoft
// https://www.manusoft.com
// All Rights Reserved
//
// Use of this software is governed by the terms of:
// GNU General Public License v2.0
// See LICENSE for details

#pragma once

// Custom heap for allocating dynamic executable code objects

#ifndef _CODEHEAP_H_
#define _CODEHEAP_H_

#ifndef HEAP_CREATE_ENABLE_EXECUTE
#define HEAP_CREATE_ENABLE_EXECUTE 0x00040000
#endif

#include <stdlib.h>

class CCodeHeap
{
	HANDLE mhHeap;
	static DWORD GetHeapCharacteristics()
		{
		#ifdef HEAP_CREATE_ENABLE_EXECUTE
			DWORD dwVersion = GetVersion();
			if( LOBYTE(LOWORD(dwVersion)) > 5 || (LOBYTE(LOWORD(dwVersion)) == 5 && HIBYTE(LOWORD(dwVersion)) >= 2) )
				return (HEAP_CREATE_ENABLE_EXECUTE | HEAP_GENERATE_EXCEPTIONS);
			else
				return (HEAP_GENERATE_EXCEPTIONS);
		#else
			return (HEAP_GENERATE_EXCEPTIONS);
		#endif
		}
	static DWORD GetHeapMaxSize()
		{
		#ifdef _DEBUG
			return 0x60000;
		#else
			return 0;
		#endif
		}
public:
	CCodeHeap()
		: mhHeap( HeapCreate( GetHeapCharacteristics(), 0, GetHeapMaxSize() ) )
		{
			if( mhHeap )
			{
				typedef BOOL (WINAPI *_F_HeapSetInformation)( HANDLE, INT, PVOID, SIZE_T );
				static _F_HeapSetInformation pfSetHeapInformation =
					(_F_HeapSetInformation)GetProcAddress( GetModuleHandleA( "KERNEL32.DLL" ), "HeapSetInformation" );
				if( pfSetHeapInformation )
				{
					static ULONG ulLowFragHeap = 2;
					pfSetHeapInformation( mhHeap, 0, &ulLowFragHeap, sizeof(ulLowFragHeap) );
				}
			}
		}
	virtual ~CCodeHeap()
		{
			if( mhHeap )
				HeapDestroy( mhHeap );
		}
	LPVOID Alloc( SIZE_T cbBlock )
		{
			LPVOID pvMem = HeapAlloc( mhHeap, 0, cbBlock );
		#ifdef _DEBUG
			if( pvMem == NULL )
			{
				MEMORYSTATUS stat;
				stat.dwLength = sizeof (stat);
				GlobalMemoryStatus (&stat);
				BOOL bValid = HeapValidate( mhHeap, 0, NULL );
				bValid;
				DebugBreak();
			}
		#endif //_DEBUG
			return pvMem;
		}
	VOID Free( LPVOID pvMem )
		{
			HeapFree( mhHeap, 0, pvMem );
		}
	HANDLE GetHeapHandle() const { return mhHeap; }

	//memory allocation
	static void* operator new( size_t n ) { return malloc( n ); }
	static void* operator new[]( size_t n ) { return malloc( n ); }
	static void operator delete( void* p ) { free( p ); }
	static void operator delete[]( void* p ) { free( p ); }
};

#endif //_CODEHEAP_H_
