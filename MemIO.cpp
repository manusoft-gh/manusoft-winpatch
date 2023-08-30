// Copyright 2010 ManuSoft
// https://www.manusoft.com
// All Rights Reserved
//
// Use of this software is governed by the terms of:
// GNU General Public License v2.0
// See LICENSE for details

#include "StdAfx.h"
#include "MemIO.h"


static DWORD CombineProtectFlags( DWORD dwDesired, DWORD dwCurrent )
{
	if( dwDesired == dwCurrent )
		return dwDesired;
	if( dwCurrent == PAGE_NOACCESS )
		return dwDesired;
	switch( dwDesired )
	{
	case PAGE_READONLY:
		switch( dwCurrent )
		{
		case PAGE_READWRITE:
		case PAGE_WRITECOPY:
		case PAGE_EXECUTE_READ:
		case PAGE_EXECUTE_READWRITE:
		case PAGE_EXECUTE_WRITECOPY:
			return dwCurrent;
		case PAGE_EXECUTE:
			return PAGE_EXECUTE_READ;
		}
	case PAGE_READWRITE:
		switch( dwCurrent )
		{
		case PAGE_READONLY:
			return dwDesired;
		case PAGE_WRITECOPY:
		case PAGE_EXECUTE_READWRITE:
		case PAGE_EXECUTE_WRITECOPY:
			return dwCurrent;
		case PAGE_EXECUTE:
		case PAGE_EXECUTE_READ:
			return PAGE_EXECUTE_READWRITE;
		}
	case PAGE_WRITECOPY:
		switch( dwCurrent )
		{
		case PAGE_READONLY:
		case PAGE_READWRITE:
			return dwDesired;
		case PAGE_EXECUTE:
		case PAGE_EXECUTE_READ:
		case PAGE_EXECUTE_READWRITE:
		case PAGE_EXECUTE_WRITECOPY:
			return PAGE_EXECUTE_WRITECOPY;
		}
	case PAGE_EXECUTE:
		switch( dwCurrent )
		{
		case PAGE_READONLY:
			return PAGE_EXECUTE_READ;
		case PAGE_READWRITE:
			return PAGE_EXECUTE_READWRITE;
		case PAGE_WRITECOPY:
			return PAGE_EXECUTE_WRITECOPY;
		case PAGE_EXECUTE_READ:
		case PAGE_EXECUTE_READWRITE:
		case PAGE_EXECUTE_WRITECOPY:
			return dwCurrent;
		}
	case PAGE_EXECUTE_READWRITE:
		switch( dwCurrent )
		{
		case PAGE_READONLY:
		case PAGE_READWRITE:
		case PAGE_EXECUTE_READ:
			return dwDesired;
		case PAGE_WRITECOPY:
			return PAGE_EXECUTE_WRITECOPY;
		case PAGE_EXECUTE_READWRITE:
		case PAGE_EXECUTE_WRITECOPY:
			return dwCurrent;
		}
	}
	return dwDesired;
}

MemProtectOverride::MemProtectOverride( LPCVOID pvTarget,
																				const size_t cbRange,
																				const DWORD dwProtect /*= PAGE_READWRITE*/ )
:	m_pmbHead( NULL ),
	m_pmbTail( NULL )
{
	if( !Set( pvTarget, cbRange, dwProtect ) )
		throw 1;
}

MemProtectOverride::~MemProtectOverride() THROW_SPEC
{
	if( !Revert() )
		throw 1;
}

bool MemProtectOverride::Set( LPCVOID pvTarget,
															const size_t cbRange,
															const DWORD dwProtect /*= PAGE_READWRITE*/ )
{
	LPVOID pvEnd = (BYTE*)pvTarget + cbRange;
	size_t cbRemaining = cbRange;
	MEMORY_BASIC_INFORMATION mbiCurrent;
	do
	{
		LPVOID pvStart = (BYTE*)pvEnd - cbRemaining;
		if( VirtualQuery( pvStart, &mbiCurrent, sizeof(mbiCurrent) ) != sizeof(mbiCurrent) )
		{
			Revert();
			return false;
		}
		MemBlock* pmbNew =
			new MemBlock( mbiCurrent.BaseAddress, mbiCurrent.RegionSize, mbiCurrent.Protect );
		LPVOID& pvBase = pmbNew->pvBase;
		size_t& cbSize = pmbNew->cbSize;
		DWORD& dwOldProtect = pmbNew->dwProtect;
		if( pvEnd > (BYTE*)pvBase + cbSize )
			cbRemaining = (BYTE*)pvEnd - ((BYTE*)pvBase + cbSize);
		else
		{
			cbRemaining = 0;
			cbSize = (BYTE*)pvEnd - (BYTE*)pvBase;
		}
		if( pvBase < pvStart )
		{
			size_t cbDelta = ((BYTE*)pvStart - (BYTE*)pvBase);
			pvBase = (LPVOID)((BYTE*)pvBase + cbDelta);
			cbSize -= cbDelta;
		}
		DWORD dwNewProtect = CombineProtectFlags( dwProtect, dwOldProtect );
		//Optimization: if nothing changes, or changing to PAGE_READONLY and already
		//have read access, do nothing
		if( dwNewProtect != dwOldProtect )
		{
			DWORD dwOldState;
			if( !VirtualProtect( pvBase, cbSize,  dwNewProtect, &dwOldState ) )
			{
				Revert();
				return false;
			}
		}
		if( NULL == m_pmbHead )
			m_pmbHead = m_pmbTail = pmbNew;
		else
		{
			m_pmbTail->pNext = pmbNew;
			m_pmbTail = pmbNew;
		}
	}
	while( cbRemaining > 0 );
	return true;
}

bool MemProtectOverride::Revert()
{
	bool bSuccess = true;
	MemBlock* pmbCursor = m_pmbHead;
	while( pmbCursor )
	{
		DWORD dwOldState;
		if( !VirtualProtect( pmbCursor->pvBase, pmbCursor->cbSize, pmbCursor->dwProtect, &dwOldState ) )
			bSuccess = false;
		MemBlock* pmbNext = pmbCursor->pNext;
		delete pmbCursor;
		pmbCursor = pmbNext;
	}
	m_pmbHead = m_pmbTail = NULL;
	return bSuccess;
}


bool SafeMemCpy( LPVOID pvTarget, LPCVOID pvSource, size_t cbRange )
{
	if( cbRange <= 0 )
		return false;
	try
	{
		MemProtectOverride MemStateSrc( pvSource, cbRange );
		MemProtectOverride MemStateDest( pvTarget, cbRange );
		MoveMemory( pvTarget, pvSource, cbRange );
	}
	catch( ... )
	{
		return false;
	}
	return true;
}


bool SafeMemFill( LPVOID pvTarget, size_t cbRange, CONST BYTE cFill )
{
	if( cbRange <= 0 )
		return true;
	try
	{
		MemProtectOverride MemStateDest( pvTarget, cbRange );
		FillMemory( pvTarget, cbRange, cFill );
	}
	catch( ... )
	{
		return false;
	}
	return true;
}


bool SetMemExecutable( LPVOID pvTarget, size_t cbRange )
{
	if( cbRange <= 0 )
		return true;
	MEMORY_BASIC_INFORMATION mbiCurrent;
	if( VirtualQuery( pvTarget, &mbiCurrent, sizeof(mbiCurrent) ) != sizeof(mbiCurrent) )
		return false;
	DWORD dwNewProtect = CombineProtectFlags( PAGE_EXECUTE, mbiCurrent.Protect );
	if( dwNewProtect != mbiCurrent.Protect )
	{
		DWORD dwOldState;
		if( !VirtualProtect( pvTarget, cbRange, dwNewProtect, &dwOldState ) )
			return false;
	}
	return true;
}
