// Copyright 2010 ManuSoft
// https://www.manusoft.com
// All Rights Reserved
//
// Use of this software is governed by the terms of:
// GNU General Public License v2.0
// See LICENSE for details

#pragma once

#ifndef _MEMIO_H_
#define _MEMIO_H_


#ifndef DLLEXPORT
#define DLLEXPORT
#endif

#if (_MSC_VER >= 1600)
#define THROW_SPEC throw(...)
#else
#define THROW_SPEC
#endif


DLLEXPORT bool SafeMemCpy( LPVOID pvTarget, LPCVOID pvSource, size_t cbRange );
DLLEXPORT bool SafeMemFill( LPVOID pvTarget, size_t cbRange, CONST BYTE cFill );
DLLEXPORT bool SetMemExecutable( LPVOID pvTarget, size_t cbRange );

class DLLEXPORT MemProtectOverride
{
	struct MemBlock
	{
		LPVOID pvBase;
		size_t cbSize;
		DWORD dwProtect;
		MemBlock* pNext;
		MemBlock( LPVOID pvBase_, size_t cbSize_, DWORD dwProtect_, MemBlock* pNext_ = NULL )
			:	pvBase( pvBase_ ), cbSize( cbSize_ ), dwProtect( dwProtect_ ), pNext( pNext_ ) {}
	};
	MemBlock* m_pmbHead;
	MemBlock* m_pmbTail;
	bool Set( LPCVOID pvTarget,
						const size_t cbRange,
						const DWORD dwProtect = PAGE_READWRITE );
	bool Revert();
public:
	MemProtectOverride( LPCVOID pvTarget,
											const size_t cbRange,
											const DWORD dwProtect = PAGE_READWRITE );
	virtual ~MemProtectOverride() THROW_SPEC;
};

template < class Tx >  // Read value from safe address
Tx Peek( const Tx& Target )
{
	MemProtectOverride MemState( (LPVOID)&Target, sizeof(Tx), PAGE_READONLY );
	return Target;
}

template < class Tx >  // Read value from safe address
Tx Peek( LPVOID pvTarget )
{
	MemProtectOverride MemState( pvTarget, sizeof(Tx), PAGE_READONLY );
	return *(Tx*)pvTarget;
}

template < class Tx >  // Stuff value into safe address
void Poke( LPVOID pvTarget, const Tx& NewValue )
{
	MemProtectOverride MemState( pvTarget, sizeof(Tx) );
	*((Tx*)pvTarget) = NewValue;
	return;
}

#endif //_MEMIO_H_
