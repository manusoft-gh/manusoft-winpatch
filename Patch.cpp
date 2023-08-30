// Copyright 2010 ManuSoft
// https://www.manusoft.com
// All Rights Reserved
//
// Use of this software is governed by the terms of:
// GNU General Public License v2.0
// See LICENSE for details

#include "StdAfx.h"
#include "MemIO.h"
#include "Patch.h"


PatchBackup::PatchBackup( LPVOID pvTarget, ULONG cbPatch )
: m_pvBackup( pvTarget? new BYTE[cbPatch] : NULL ),
	m_pvTarget( pvTarget ),
	m_cbPatch( cbPatch )
{
	if( !m_pvBackup )
		throw 1;
	MemProtectOverride MemState( pvTarget, cbPatch, PAGE_READONLY );
	MoveMemory( m_pvBackup, pvTarget, cbPatch );
}


PatchBackup::~PatchBackup()
{
	Unpatch();
	delete[]( m_pvBackup );
}


bool PatchBackup::Unpatch() const
{
	try
	{
		MemProtectOverride MemState( m_pvTarget, m_cbPatch, PAGE_READWRITE );
		MoveMemory( m_pvTarget, m_pvBackup, m_cbPatch );
	}
	catch( ... )
	{
		return false;
	}
	return true;
}


Patch::Patch( LPVOID pvTarget, ULONG cbPatch, LPCVOID pvSource, bool bDelay /*= false*/ )
: m_pvSource( NULL ),
	m_Backup( pvTarget, cbPatch )
{
	if( !Init( pvSource ) )
		throw 1;
	if( bDelay )
		return;
	if( !PatchNow() )
		throw 2;
	return;
}


Patch::~Patch()
{
	UnpatchNow();
	delete[]( m_pvSource );
}


bool Patch::PatchNow() const
{
	if( !m_pvSource )
		return false;
	return SafeMemCpy( GetTarget(), m_pvSource, GetSize() );
}


bool Patch::Init( LPCVOID pvSource )
{
	LPVOID pvSourceCopy = new BYTE[GetSize()];
	if( !pvSourceCopy )
		return false;
	if( !SafeMemCpy( pvSourceCopy, pvSource, GetSize() ) )
		return false;
	m_pvSource = pvSourceCopy;
	return true;
}
