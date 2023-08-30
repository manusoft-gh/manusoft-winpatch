// Copyright 2010 ManuSoft
// https://www.manusoft.com
// All Rights Reserved
//
// Use of this software is governed by the terms of:
// GNU General Public License v2.0
// See LICENSE for details

#pragma once

#ifndef _EXPORTEDFUNCTIONHOOK_H_
#define _EXPORTEDFUNCTIONHOOK_H_

#include "Patch.h"
#include "PeImage.h"


#ifndef DLLEXPORT
#define DLLEXPORT
#endif


//Main hook class
class DLLEXPORT ExportedFunctionHook
{
protected:
	HMODULE m_hmodTarget;
	PatchT<RVA>* m_pThisHook;
	FARPROC m_pfOriginalFunction;
	FARPROC m_pfHookFunction;

	bool Initialize( const LPCSTR pszFunction, FARPROC pHookFunc );
public:
	bool			IsHooked() const
							{ return (NULL != m_pThisHook); }
	LPVOID			ChainOriginal() const
							{ return m_pfOriginalFunction; }
	FARPROC		HookFunction() const
							{ return m_pfHookFunction; }
						ExportedFunctionHook( const LPCTSTR pszTargetModule,
																	const LPCSTR pszFunction,
																	FARPROC pHookFunc );
						ExportedFunctionHook( const PeImage& PeTarget,
																	const LPCSTR pszFunction,
																	FARPROC pHookFunc );
						ExportedFunctionHook( const LPCTSTR pszTargetModule,
																	const DWORD dwFunctionOrdinal,
																	FARPROC pHookFunc );
						ExportedFunctionHook( const PeImage& PeTarget,
																	const DWORD dwFunctionOrdinal,
																	FARPROC pHookFunc );
	virtual		~ExportedFunctionHook();
};


#endif //_EXPORTEDFUNCTIONHOOK_H_
