// Copyright 2010 ManuSoft
// https://www.manusoft.com
// All Rights Reserved
//
// Use of this software is governed by the terms of:
// GNU General Public License v2.0
// See LICENSE for details

#include "StdAfx.h"
#include "ImportedFunctionHook.h"


bool ImportedFunctionDelayedHook::HookNow()
{
	unhook();
	mhmodTarget = ::GetModuleHandleA( mszImportingModule );
	if( mhmodTarget && mszExportingModule )
	{
		LPCSTR pszTarget = ((mdwFunctionOrdinal == ~0)? mszFunction : (LPCSTR)(DWORD_PTR)mdwFunctionOrdinal);
		mpfOriginal = ::GetProcAddress( ::GetModuleHandleA( mszExportingModule ), pszTarget );
		mppfTarget = PeGetImportTableEntry( mhmodTarget, mszExportingModule, pszTarget );
	}
	return hook( mpfHook );
}
