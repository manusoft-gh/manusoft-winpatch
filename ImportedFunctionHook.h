// Copyright 2010 ManuSoft
// https://www.manusoft.com
// All Rights Reserved
//
// Use of this software is governed by the terms of:
// GNU General Public License v2.0
// See LICENSE for details

#pragma once

#ifndef _IMPORTEDFUNCTIONHOOK_H_
#define _IMPORTEDFUNCTIONHOOK_H_

#include "FunctionTableHook.h"
#include "PeImage.h"
#include "PeUtility.h"


#ifndef DLLEXPORT
#define DLLEXPORT
#endif


//Function hook macros
#define MAINMODULE NULL

#define IFHOOKMAIN(Mod,Func)\
	ImportedFunctionHook ifhkMain_##Func( IFTE(#Mod, MAINMODULE, #Func), (FARPROC)(My##Func) )

#define IFHOOKMAINALIAS(Mod,Alias,Func)\
	ImportedFunctionHook ifhkMain_##Alias( IFTE(#Mod, MAINMODULE, #Func), (FARPROC)(My##Alias) )

#define IFHOOKMAIN_BYORDINAL(Mod,Func,Ord)\
	ImportedFunctionHook ifhkMain_##Func( IFTE(#Mod, MAINMODULE, (DWORD)((WORD)(Ord))), (FARPROC)(My##Func) )

#define IFHOOKDLL(SrcMod,TargetMod,Func)\
	ImportedFunctionHook ifhk##TargetMod##_##Func( IFTE(#SrcMod, #TargetMod".DLL", #Func), (FARPROC)(My##Func) )

#define IFHOOKDLLALIAS(SrcMod,DestMod,Alias,Func)\
	ImportedFunctionHook ifhk##DestMod##_##Alias( IFTE(#SrcMod, #DestMod".DLL", #Func), (FARPROC)(My##Alias) )

#define IFHOOKDLL_BYORDINAL(SrcMod,TargetMod,Func,Ord)\
	ImportedFunctionHook ifhk##TargetMod##_##Func( IFTE(#SrcMod, #TargetMod".DLL", (DWORD)((WORD)(Ord))), (FARPROC)(My##Func) )

#define IFHOOKDLL_EXT(SrcMod,TargetMod,Ext,Func)\
	ImportedFunctionHook ifhk##TargetMod##Ext##_##Func( IFTE(#SrcMod, #TargetMod"."#Ext, #Func), (FARPROC)(My##Func) )

#define IFHOOKDLL_EXTALIAS(SrcMod,TargetMod,Ext,Alias,Func)\
	ImportedFunctionHook ifhk##TargetMod##Ext##_##Alias( IFTE(#SrcMod, #TargetMod"."#Ext, #Func), (FARPROC)(My##Alias) )

#define IFHOOKDLL_EXT_BYORDINAL(SrcMod,TargetMod,Ext,Func,Ord)\
	ImportedFunctionHook ifhk##TargetMod##Ext##_##Func( IFTE(#SrcMod, #TargetMod"."#Ext, (DWORD)((WORD)(Ord))), (FARPROC)(My##Func) )

#define IFDLYHOOKDLL(SrcMod,TargetMod,Func)\
	ImportedFunctionDelayedHook ifhk##TargetMod##_##Func( #SrcMod, #TargetMod".DLL", #Func, (FARPROC)(My##Func) )

#define IFDLYHOOKDLLALIAS(SrcMod,DestMod,Alias,Func)\
	ImportedFunctionDelayedHook ifhk##DestMod##_##Alias( #SrcMod, #DestMod".DLL", #Func, (FARPROC)(My##Alias) )

#define IFDLYHOOKDLL_BYORDINAL(SrcMod,TargetMod,Func,Ord)\
	ImportedFunctionDelayedHook ifhk##TargetMod##_##Func( #SrcMod, #TargetMod".DLL", (DWORD)((WORD)(Ord)), (FARPROC)(My##Func) )

#define IFDLYHOOKDLL_EXT(SrcMod,TargetMod,Ext,Func)\
	ImportedFunctionDelayedHook ifhk##TargetMod##Ext##_##Func( #SrcMod, #TargetMod"."#Ext, #Func, (FARPROC)(My##Func) )

#define IFDLYHOOKDLL_EXTALIAS(SrcMod,TargetMod,Ext,Alias,Func)\
	ImportedFunctionDelayedHook ifhk##TargetMod##Ext##_##Alias( #SrcMod, #TargetMod"."#Ext, #Func, (FARPROC)(My##Alias) )

#define IFDLYHOOKDLL_EXT_BYORDINAL(SrcMod,TargetMod,Ext,Func,Ord)\
	ImportedFunctionDelayedHook ifhk##TargetMod##Ext##_##Func( #SrcMod, #TargetMod"."#Ext, (DWORD)((WORD)(Ord)), (FARPROC)(My##Func) )


class DLLEXPORT ImportedFunctionTableEntry
{
protected:
	HMODULE mhmodTarget;
	FARPROC* mppfTarget;
	FARPROC mpfPrevious;
	FARPROC mpfOriginal;

public:
	ImportedFunctionTableEntry( const LPCSTR pszExportingModule,
															const LPCSTR pszImportingModule,
															const LPCSTR pszFunction )
		: mhmodTarget( ::GetModuleHandleA( pszImportingModule ) )
		, mppfTarget( NULL )
		, mpfPrevious( NULL )
		, mpfOriginal( NULL )
		{
			if( mhmodTarget && pszExportingModule && (HIWORD(pszFunction) == 0 || pszFunction[0] != '\0') )
			{
				mpfOriginal = ::GetProcAddress( ::GetModuleHandleA( pszExportingModule ), pszFunction );
				mppfTarget = PeGetImportTableEntry( mhmodTarget, pszExportingModule, pszFunction );
				if( mppfTarget )
					mpfPrevious = *mppfTarget;
			}
		}
	ImportedFunctionTableEntry( const LPCSTR pszExportingModule,
															const PeImage& PeImportingModule,
															const LPCSTR pszFunction )
		: mhmodTarget( PeImportingModule )
		, mppfTarget( NULL )
		, mpfPrevious( NULL )
		, mpfOriginal( NULL )
		{
			if( mhmodTarget && pszExportingModule && (HIWORD(pszFunction) == 0 || pszFunction[0] != '\0') )
			{
				mpfOriginal = ::GetProcAddress( ::GetModuleHandleA( pszExportingModule ), pszFunction );
				mppfTarget = PeGetImportTableEntry( mhmodTarget, pszExportingModule, pszFunction );
				if( mppfTarget )
					mpfPrevious = *mppfTarget;
			}
		}
	ImportedFunctionTableEntry( const LPCSTR pszExportingModule,
															const LPCSTR pszImportingModule,
															const DWORD dwFunctionOrdinal )
		: mhmodTarget( ::GetModuleHandleA( pszImportingModule ) )
		, mppfTarget( NULL )
		, mpfPrevious( NULL )
		, mpfOriginal( NULL )
		{
			if( mhmodTarget && pszExportingModule )
			{
				mpfOriginal = ::GetProcAddress( ::GetModuleHandleA( pszExportingModule ), (LPCSTR)(DWORD_PTR)dwFunctionOrdinal );
				mppfTarget = PeGetImportTableEntry( mhmodTarget, pszExportingModule, dwFunctionOrdinal );
				if( mppfTarget )
					mpfPrevious = *mppfTarget;
			}
		}
	ImportedFunctionTableEntry( const LPCSTR pszExportingModule,
															const PeImage& PeImportingModule,
															const DWORD dwFunctionOrdinal )
		: mhmodTarget( PeImportingModule )
		, mppfTarget( NULL )
		, mpfPrevious( NULL )
		, mpfOriginal( NULL )
		{
			if( mhmodTarget && pszExportingModule )
			{
				mpfOriginal = ::GetProcAddress( ::GetModuleHandleA( pszExportingModule ), (LPCSTR)(DWORD_PTR)dwFunctionOrdinal );
				mppfTarget = PeGetImportTableEntry( mhmodTarget, pszExportingModule, dwFunctionOrdinal );
				if( mppfTarget )
					mpfPrevious = *mppfTarget;
			}
		}

		HMODULE importingModule() const { return mhmodTarget; }
		FARPROC* importTableEntry() const { return mppfTarget; }
		FARPROC previousFunction() const { return mpfPrevious; }
		FARPROC originalFunction() const { return mpfOriginal; }
};
typedef ImportedFunctionTableEntry IFTE; //shortcut


//Main hook class
class DLLEXPORT ImportedFunctionHook : public FunctionTableHookBase
{
protected:
	HMODULE mhmodTarget;
	FARPROC mpfOriginal;

public:
	FARPROC chainOriginal() const { return mpfOriginal; }

	ImportedFunctionHook( const ImportedFunctionTableEntry& Target, FARPROC pfHook )
		: FunctionTableHookBase( Target.importTableEntry(), pfHook )
		, mhmodTarget( Target.importingModule() )
		, mpfOriginal( Target.originalFunction() )
		{}
};


//Hook class that passes TPtr* to hook function
template< typename TPtr >
class DLLEXPORT ImportedFunctionHookPtr : public FunctionTableHookPtr< TPtr >
{
protected:
	HMODULE mhmodTarget;
	FARPROC mpfOriginal;

public:
	FARPROC chainOriginal() const { return mpfOriginal; }

	ImportedFunctionHookPtr( const TPtr* pArg, const ImportedFunctionTableEntry& Target, FARPROC pfHook, const FunctionInfo info )
		: FunctionTableHookPtr< TPtr >( pArg, Target.importTableEntry(), info, pfHook )
		, mhmodTarget( Target.importingModule() )
		, mpfOriginal( Target.originalFunction() )
		{}
};


class DLLEXPORT ImportedFunctionDelayedHook : public ImportedFunctionHook
{
protected:
	CHAR mszExportingModule[MAX_PATH];
	CHAR mszImportingModule[MAX_PATH];
	CHAR mszFunction[MAX_PATH * 4];
	DWORD mdwFunctionOrdinal;
	FARPROC mpfHook;

public:
	ImportedFunctionDelayedHook( const LPCSTR pszExportingModule,
															 const LPCSTR pszImportingModule,
															 const LPCSTR pszFunction,
															 FARPROC pfHook )
		: ImportedFunctionHook( IFTE( pszExportingModule, pszImportingModule, pszFunction ), pfHook ),
			mdwFunctionOrdinal( ~DWORD(0) ),
			mpfHook( pfHook )
		{
			lstrcpynA( mszExportingModule, pszExportingModule, MAX_PATH );
			lstrcpynA( mszImportingModule, pszImportingModule, MAX_PATH );
			lstrcpynA( mszFunction, pszFunction, MAX_PATH * 4 );
		}
	ImportedFunctionDelayedHook( const LPCSTR pszExportingModule,
															 const LPCSTR pszImportingModule,
															 const DWORD dwFunctionOrdinal,
															 FARPROC pfHook )
		: ImportedFunctionHook( IFTE( pszExportingModule, pszImportingModule, dwFunctionOrdinal ), pfHook ),
			mdwFunctionOrdinal( dwFunctionOrdinal ),
			mpfHook( pfHook )
		{
			lstrcpynA( mszExportingModule, pszExportingModule, MAX_PATH );
			lstrcpynA( mszImportingModule, pszImportingModule, MAX_PATH );
			mszFunction[0] = '\0';
		}
	bool HookNow();
};


#endif //_IMPORTEDFUNCTIONHOOK_H_
