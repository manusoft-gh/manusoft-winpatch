// Copyright 2010 ManuSoft
// https://www.manusoft.com
// All Rights Reserved
//
// Use of this software is governed by the terms of:
// GNU General Public License v2.0
// See LICENSE for details

#pragma once

#ifndef _ACTCTX_H_
#define _ACTCTX_H_

#include "PeImage.h"

#ifndef assert
#define assert(_Expression)     ((void)0)
#endif //asert


#ifdef _WIN64
typedef unsigned __int64 ULONG_PTR;
#else
typedef unsigned long ULONG_PTR;
#endif

/**************************************************************************************/
// from WinBase.h
#if (_WIN32_WINNT >= 0x0500) || (_WIN32_FUSION >= 0x0100) || ISOLATION_AWARE_ENABLED
#else
#define ACTCTX_FLAG_PROCESSOR_ARCHITECTURE_VALID    (0x00000001)
#define ACTCTX_FLAG_LANGID_VALID                    (0x00000002)
#define ACTCTX_FLAG_ASSEMBLY_DIRECTORY_VALID        (0x00000004)
#define ACTCTX_FLAG_RESOURCE_NAME_VALID             (0x00000008)
#define ACTCTX_FLAG_SET_PROCESS_DEFAULT             (0x00000010)
#define ACTCTX_FLAG_APPLICATION_NAME_VALID          (0x00000020)
#define ACTCTX_FLAG_SOURCE_IS_ASSEMBLYREF           (0x00000040)
#define ACTCTX_FLAG_HMODULE_VALID                   (0x00000080)

typedef struct tagACTCTXA {
    ULONG       cbSize;
    DWORD       dwFlags;
    LPCSTR      lpSource;
    USHORT      wProcessorArchitecture;
    LANGID      wLangId;
    LPCSTR      lpAssemblyDirectory;
    LPCSTR      lpResourceName;
    LPCSTR      lpApplicationName;
    HMODULE     hModule;
} ACTCTXA, *PACTCTXA;

typedef const ACTCTXA *PCACTCTXA;

//WINBASEAPI __out HANDLE WINAPI CreateActCtxA( __in PCACTCTXA pActCtx );
//WINBASEAPI VOID WINAPI AddRefActCtx( __inout HANDLE hActCtx );
//WINBASEAPI VOID WINAPI ReleaseActCtx( __inout HANDLE hActCtx );
//WINBASEAPI BOOL WINAPI ActivateActCtx( __inout_opt HANDLE hActCtx, __out ULONG_PTR *lpCookie );

#define DEACTIVATE_ACTCTX_FLAG_FORCE_EARLY_DEACTIVATION (0x00000001)
//WINBASEAPI BOOL WINAPI DeactivateActCtx( __in DWORD dwFlags, __in ULONG_PTR ulCookie );
#endif
/**************************************************************************************/


class CActCtx
{
	HMODULE mhmodKernel32;
	HANDLE mhActCtx;
	ULONG_PTR mnCookie;
private:
	CActCtx( const CActCtx& rhs );
	CActCtx& operator =( const CActCtx& rhs );
public:
	CActCtx( HANDLE hActCtx )
		: mhmodKernel32( LoadLibraryA( "KERNEL32.DLL" ) )
		, mhActCtx( hActCtx )
		, mnCookie( 0 )
	{
		if( hActCtx != INVALID_HANDLE_VALUE )
		{
			AddRefActCtx( hActCtx );
			BOOL bSuccess = ActivateActCtx( hActCtx, &mnCookie );
			assert( bSuccess == TRUE );
		}
	}
	CActCtx( PeImage& CtxImage, bool bDLL = true, bool bActivate = true )
		: mhmodKernel32( LoadLibraryA( "KERNEL32.DLL" ) )
		, mhActCtx( NULL )
		, mnCookie( 0 )
	{
		mhActCtx = CreateActCtxFromImage( CtxImage, bDLL );
		if( bActivate && mhActCtx != INVALID_HANDLE_VALUE )
		{
			BOOL bSuccess = ActivateActCtx( mhActCtx, &mnCookie );
			assert( bSuccess == TRUE );
		}
	}
	virtual ~CActCtx()
	{
		if( mhActCtx != INVALID_HANDLE_VALUE )
		{
			if( mnCookie != 0 )
			{
				BOOL bSuccess = DeactivateActCtx( 0, mnCookie );
				assert( bSuccess == TRUE );
			}
			ReleaseActCtx( mhActCtx );
		}
		FreeLibrary( mhmodKernel32 );
	}
	operator HANDLE() { return mhActCtx; }
	static bool IsActCtxSupported()
	{
		static bool bSupported = (GetProcAddress( GetModuleHandle( _T("KERNEL32.DLL") ), "CreateActCtxA" ) != NULL);
		return bSupported;
	}
	static HANDLE CreateActCtxFromImage( PeImage& CtxImage, bool bDLL = true )
	{
		bool bMappedFile = CtxImage.IsMappedFile();
		ACTCTXA ActCtx = 
		{
			sizeof(ACTCTXA),
			ACTCTX_FLAG_RESOURCE_NAME_VALID | (bMappedFile? ACTCTX_FLAG_APPLICATION_NAME_VALID : ACTCTX_FLAG_HMODULE_VALID),
			bMappedFile? CtxImage.GetMappedFile()->GetModulePath() : NULL,
			0,
			0,
			NULL,
			bDLL? MAKEINTRESOURCEA(2) : MAKEINTRESOURCEA(1),
			NULL,
			bMappedFile? NULL : (HMODULE)CtxImage,
		};
		return CreateActCtxA( &ActCtx );
	}
protected:
	static HANDLE CreateActCtxA( PCACTCTXA pActCtx )
	{
		typedef HANDLE (__stdcall *CreateActCtxAProc)( PCACTCTXA );
		static CreateActCtxAProc pfCreateActCtxA =
			(CreateActCtxAProc)GetProcAddress( GetModuleHandle( _T("KERNEL32.DLL") ), "CreateActCtxA" );
		if( !pfCreateActCtxA )
		{
			SetLastError( ERROR_NOT_SUPPORTED );
			return INVALID_HANDLE_VALUE;
		}
		return pfCreateActCtxA( pActCtx );
	}
	static VOID AddRefActCtx( HANDLE hActCtx )
	{
		typedef VOID (__stdcall *AddRefActCtxProc)( HANDLE );
		static AddRefActCtxProc pfAddRefActCtx =
			(AddRefActCtxProc)GetProcAddress( GetModuleHandle( _T("KERNEL32.DLL") ), "AddRefActCtx" );
		if( !pfAddRefActCtx )
		{
			SetLastError( ERROR_NOT_SUPPORTED );
			return;
		}
		return pfAddRefActCtx( hActCtx );
	}
	static VOID ReleaseActCtx( HANDLE hActCtx )
	{
		typedef VOID (__stdcall *ReleaseActCtxProc)( HANDLE );
		static ReleaseActCtxProc pfReleaseActCtx =
			(ReleaseActCtxProc)GetProcAddress( GetModuleHandle( _T("KERNEL32.DLL") ), "ReleaseActCtx" );
		if( !pfReleaseActCtx )
		{
			SetLastError( ERROR_NOT_SUPPORTED );
			return;
		}
		return pfReleaseActCtx( hActCtx );
	}
	static BOOL ActivateActCtx( HANDLE hActCtx, ULONG_PTR* lpCookie )
	{
		typedef BOOL (__stdcall *ActivateActCtxProc)( HANDLE, ULONG_PTR* );
		static ActivateActCtxProc pfActivateActCtx =
			(ActivateActCtxProc)GetProcAddress( GetModuleHandle( _T("KERNEL32.DLL") ), "ActivateActCtx" );
		if( !pfActivateActCtx )
		{
			SetLastError( ERROR_NOT_SUPPORTED );
			return FALSE;
		}
		return pfActivateActCtx( hActCtx, lpCookie );
	}
	static BOOL DeactivateActCtx( DWORD dwFlags, ULONG_PTR ulCookie )
	{
		typedef BOOL (__stdcall *DeactivateActCtxProc)( DWORD, ULONG_PTR );
		static DeactivateActCtxProc pfDeactivateActCtx =
			(DeactivateActCtxProc)GetProcAddress( GetModuleHandle( _T("KERNEL32.DLL") ), "DeactivateActCtx" );
		if( !pfDeactivateActCtx )
		{
			SetLastError( ERROR_NOT_SUPPORTED );
			return FALSE;
		}
		return pfDeactivateActCtx( dwFlags, ulCookie );
	}
};

#endif //_ACTCTX_H_
