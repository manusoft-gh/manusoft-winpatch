// Copyright 2010 ManuSoft
// https://www.manusoft.com
// All Rights Reserved
//
// Use of this software is governed by the terms of:
// GNU General Public License v2.0
// See LICENSE for details

#pragma once

#ifndef _PATCH_H_
#define _PATCH_H_

#include "MemIO.h"


#ifndef DLLEXPORT
#define DLLEXPORT
#endif


class DLLEXPORT PatchBackup
{
protected:
	LPVOID m_pvTarget;
	LPVOID m_pvBackup;
	ULONG m_cbPatch;
public:
	LPVOID GetBackup() const { return m_pvBackup; }
	LPVOID GetTarget() const { return m_pvTarget; }
	ULONG GetSize() const { return m_cbPatch; }

	PatchBackup( LPVOID pvTarget, ULONG cbPatch );
	virtual	~PatchBackup();

	bool Unpatch() const;
};

template < class Tx >
class PatchBackupT : public PatchBackup
{
public:
	Tx* GetBackup() const { return (Tx*)m_pvBackup; }
	Tx* GetTarget() const { return (Tx*)m_pvTarget; }

	PatchBackupT( const Tx* pTarget ) : PatchBackup( pTarget, sizeof(Tx) ) {}
};


class DLLEXPORT Patch
{
protected:
	PatchBackup m_Backup;
	LPVOID	m_pvSource;

	bool Init( LPCVOID pvSource );
public:
	LPVOID GetBackup() const { return m_Backup.GetBackup(); }
	LPVOID GetTarget() const { return m_Backup.GetTarget(); }
	ULONG GetSize() const { return m_Backup.GetSize(); }

	Patch( LPVOID pvTarget, ULONG cbPatch, LPCVOID pvSource, bool bDelay = false );
	virtual	~Patch();

	bool PatchNow() const;
	bool UnpatchNow() const { return m_Backup.Unpatch(); }
};

template < class Tx >
class PatchT : public Patch
{
public:
	Tx* GetBackup() const { return (Tx*)m_Backup.GetBackup(); }
	Tx* GetTarget() const { return (Tx*)m_Backup.GetTarget(); }

	PatchT( Tx* pTarget, const Tx& Source, bool bDelay = false )
		: Patch( pTarget, sizeof(Tx), (LPVOID)&Source, bDelay ) {};
};


#endif //_PATCH_H_
