// Copyright 2010 ManuSoft
// https://www.manusoft.com
// All Rights Reserved
//
// Use of this software is governed by the terms of:
// GNU General Public License v2.0
// See LICENSE for details

#pragma once

// Implements a safe WndProcHook that cleans up after itself if
// possible, or leaves a stub behind if the WndProc has been hooked
// by someone else in the meantime.

#ifndef _WNDPROCHOOK_H_
#define _WNDPROCHOOK_H_


#ifndef DLLEXPORT
#define DLLEXPORT
#endif

#ifndef GetWindowLongPtr
#define LONG_PTR LONG
#define GetWindowLongPtrW GetWindowLongW
#define GetWindowLongPtrA GetWindowLongA
#define SetWindowLongPtrW SetWindowLongW
#define SetWindowLongPtrA SetWindowLongA
#define GWLP_WNDPROC        GWL_WNDPROC
#define GWLP_HINSTANCE      GWL_HINSTANCE
#define GWLP_HWNDPARENT     GWL_HWNDPARENT
#define GWLP_USERDATA       GWL_USERDATA
#define GWLP_ID             GWL_ID
#endif //GetWindowLongPtr

#include "Asm.h"
#include <new.h>
#include <TChar.h>

static bool WPHSetClassInfo( const WNDCLASS* pstClassInfo );


//Window Procedure functor
class CWindowProc
{
	HWND mhWnd;
	WNDPROC mpfWndProc;
public:
	CWindowProc( const HWND hWnd, const WNDPROC pfWndProc ) : mhWnd( hWnd ), mpfWndProc( pfWndProc ) {}
	LRESULT operator () ( UINT nMsg, WPARAM wParam, LPARAM lParam )
		{
			if( mpfWndProc )
				return ::CallWindowProc( mpfWndProc, mhWnd, nMsg, wParam, lParam );
			return ::DefWindowProc( mhWnd, nMsg, wParam, lParam );
		}
};


//Router stub that creates a relief valve in global memory that can be left behind 
//if there's a risk of breaking a hook chain when a hook is removed. This stub can 
//either route window messages to the hook procedure or pass them on to another 
//handler via CallWindowProc.
DLLEXPORT class WndProcStubBase
{
	typedef LRESULT (__stdcall* F_CallWindowProc)( WNDPROC, HWND, UINT, WPARAM, LPARAM );
	#ifdef _WIN64
	class AsmCallWindowProc
	{
	private:
	#pragma pack(push)
	#pragma pack(1)
		AsmPopA m_PopRetAddress;
		AsmMovIRspXRax m_SaveRetAddr;
		AsmSubRspX m_AddSpace;
		AsmMovIRspXR9 m_ShiftArg4;
		AsmMovR9R8 m_ShiftArg3;
		AsmMovR8Rdx m_ShiftArg2;
		AsmMovRdxRcx m_ShiftArg1;
		AsmMovRcxX m_WndProcPtr;
		AsmPtrCall m_CallWindowProc;
		AsmAddRspX m_AdjustStack;
		AsmPushRax m_SaveRax;
		AsmMovRaxIRspX m_GetRetAddr;
		AsmXchgRaxIRsp m_SetupRet;
		AsmRet m_Return;
	#pragma pack(pop)
	public:
		AsmCallWindowProc( WNDPROC wprocTarget, F_CallWindowProc pfCallWindowProcAddress )
			: m_SaveRetAddr( 32 )
			, m_AddSpace( 16 )
			, m_ShiftArg4( 32 )
			, m_WndProcPtr( (DWORD_PTR)wprocTarget )
			, m_CallWindowProc( pfCallWindowProcAddress )
			, m_AdjustStack( 16 )
			, m_GetRetAddr( 40 )
			{}
		void SetWndProc( WNDPROC wprocTarget ) { m_WndProcPtr.Set( (DWORD_PTR)wprocTarget ); }
	};
	#else //_WIN64
	class AsmCallWindowProc
	{
	private:
	#pragma pack(push)
	#pragma pack(1)
		AsmPopA m_SaveRetAddress;
		AsmPushPtr m_PushWndProc;
		AsmPushA m_PushRetAddress;
		AsmPtrJmp m_JmpCallWindowProc;
	#pragma pack(pop)
	public:
		AsmCallWindowProc( WNDPROC wprocTarget, F_CallWindowProc pfCallWindowProcAddress )
			: m_PushWndProc( (DWORD_PTR)wprocTarget )
			, m_JmpCallWindowProc( pfCallWindowProcAddress )
			{}
		void SetWndProc( WNDPROC wprocTarget ) { m_PushWndProc.Set( (DWORD_PTR)wprocTarget ); }
	};
	#endif //_WIN64
private:
	AsmCallWindowProc* m_pRouter;
public:
	WndProcStubBase( WNDPROC wprocTarget, F_CallWindowProc pfCallWindowProcAddress );
	virtual ~WndProcStubBase();
	bool SetTarget( WNDPROC wprocTarget );
	operator WNDPROC() const { return (WNDPROC)m_pRouter; }
};

class WndProcStubA : public WndProcStubBase
{
public:
	WndProcStubA( WNDPROC wprocTarget ) : WndProcStubBase( wprocTarget, &CallWindowProcA ) {}
};

class WndProcStubW : public WndProcStubBase
{
public:
	WndProcStubW( WNDPROC wprocTarget ) : WndProcStubBase( wprocTarget, &CallWindowProcW ) {}
};

#ifdef _UNICODE
typedef WndProcStubW WndProcStub;
#else
typedef WndProcStubA WndProcStub;
#endif


template<class C>
class ThisWndProcStub
{
private:
	#ifdef _WIN64
	struct PushThisPtrAndJumpToStub
	{
	#pragma pack(push)
	#pragma pack(1)
		AsmPopA m_PopRetAddress;
		AsmMovIRspXRax m_SaveRetAddr;
		AsmSubRspX m_AddSpace;
		AsmMovIRspXR9 m_ShiftArg4;
		AsmMovR9R8 m_ShiftArg3;
		AsmMovR8Rdx m_ShiftArg2;
		AsmMovRdxRcx m_ShiftArg1;
		AsmMovRcxX m_ThisPtr;
		AsmPtrCall m_Call;
		AsmAddRspX m_AdjustStack;
		AsmPushRax m_SaveRax;
		AsmMovRaxIRspX m_GetRetAddr;
		AsmXchgRaxIRsp m_SetupRet;
		AsmRet m_Return;
	#pragma pack(pop)
		PushThisPtrAndJumpToStub( const C* pThis, LPVOID pfWndProc )
			: m_SaveRetAddr( 32 )
			, m_AddSpace( 16 )
			, m_ShiftArg4( 32 )
			, m_ThisPtr( (DWORD_PTR)pThis )
			, m_Call( pfWndProc )
			, m_AdjustStack( 16 )
			, m_GetRetAddr( 40 )
			{}
	};
	#else
	struct PushThisPtrAndJumpToStub
	{
	#pragma pack(push)
	#pragma pack(1)
		AsmPopA m_PopEax;
		AsmPushPtr m_PushThisPtr;
		AsmPushA m_PushRetAddr;
		AsmPtrJmp m_Jmp;
	#pragma pack(pop)
		PushThisPtrAndJumpToStub( const C* pThis, LPVOID pfWndProc )
			: m_PushThisPtr( (DWORD_PTR)pThis ), m_Jmp( pfWndProc ) {}
	};
	#endif //_WIN64
	PushThisPtrAndJumpToStub* m_pPushThisPtrAndJumpToStub;
public:
	ThisWndProcStub( const C* pThis );
	~ThisWndProcStub() { VirtualFree( m_pPushThisPtrAndJumpToStub, sizeof(*m_pPushThisPtrAndJumpToStub), MEM_DECOMMIT ); }
	static LRESULT CALLBACK WindowProcStub( C* pThis, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	operator WNDPROC() const { return (WNDPROC)m_pPushThisPtrAndJumpToStub; }
	void SetWndProc( WNDPROC wprocTarget ) { m_pPushThisPtrAndJumpToStub->SetTarget( wprocTarget ); }
};


//Hook an individual window
DLLEXPORT class SafeWndProcHookA
{
private:
	WndProcStubA* m_pWndProcStub;
protected:
	HWND m_hwndTarget;
	WNDPROC m_wprocOriginal;
public:
	SafeWndProcHookA( HWND hwndTarget, WNDPROC wprocHook );
	virtual ~SafeWndProcHookA();
	virtual bool IsSafeToRemoveWndProc();
	LRESULT Chain( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) const;
	operator WNDPROC() const { return m_pWndProcStub? (WNDPROC)*m_pWndProcStub : NULL; }
	void SetWndProc( WNDPROC wprocTarget );
	HWND GetWnd() const { return m_hwndTarget; }
	WNDPROC GetChainProc() const { return m_wprocOriginal; }
};

DLLEXPORT class SafeWndProcHookW
{
private:
	WndProcStubW* m_pWndProcStub;
protected:
	HWND m_hwndTarget;
	WNDPROC m_wprocOriginal;
public:
	SafeWndProcHookW( HWND hwndTarget, WNDPROC wprocHook );
	virtual ~SafeWndProcHookW();
	virtual bool IsSafeToRemoveWndProc();
	LRESULT Chain( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) const;
	operator WNDPROC() const { return m_pWndProcStub? (WNDPROC)*m_pWndProcStub : NULL; }
	void SetWndProc( WNDPROC wprocTarget );
	HWND GetWnd() const { return m_hwndTarget; }
	WNDPROC GetChainProc() const { return m_wprocOriginal; }
};

#ifdef _UNICODE
typedef SafeWndProcHookW SafeWndProcHook;
#else
typedef SafeWndProcHookA SafeWndProcHook;
#endif


//Subclass an individual window
DLLEXPORT class SafeWindowSubclass : public SafeWndProcHook
{
private:
	ThisWndProcStub<SafeWindowSubclass> m_ThisWndProcStub;
public:
	virtual LRESULT WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) const = 0;
	SafeWindowSubclass( HWND hwndTarget )
		:	SafeWndProcHook( hwndTarget, NULL )
		, m_ThisWndProcStub( this )
		{
			SetWndProc( m_ThisWndProcStub );
		}
};


//Subclass an individual window by specifying another base class
template<class C>
class SafeThisWindowSubclass : public SafeWndProcHook
{
private:
	ThisWndProcStub<C> m_ThisWndProcStub;
public:
	SafeThisWindowSubclass( const C* pThis, HWND hwndTarget )
		: SafeWndProcHook( hwndTarget, NULL )
		, 	m_ThisWndProcStub( pThis )
		{
			SetWndProc( m_ThisWndProcStub );
		}
};


//Hook a window class (affects subsequently created windows of that class)
DLLEXPORT class SafeWindowClassHook
{
private:
	WndProcStub* m_pWndProcStub;
	WNDPROC m_wprocBaseClass;
	WNDCLASS m_stClassInfo;
public:
	SafeWindowClassHook( LPCTSTR pszTargetClass, HMODULE hmodTarget, WNDPROC wprocHook );
	virtual ~SafeWindowClassHook();
	virtual bool IsSafeToRemoveWndProc() { return false; } //no way of knowing whether any subclassed windows exist
	LRESULT Chain( HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam ) const;
	WNDCLASS& GetClass() { return m_stClassInfo; }
};


//Subclass a window class (affects subsequently created windows of that class)
DLLEXPORT class SafeWindowClassSubclass : public SafeWindowClassHook
{
private:
	ThisWndProcStub<SafeWindowClassSubclass> m_ThisWndProcStub;
public:
	virtual LRESULT WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) const
		{ return Chain( hwnd, uMsg, wParam, lParam ); }
	SafeWindowClassSubclass( LPCTSTR pszTargetClass, HMODULE hmodTarget );
};


//Create a new windows class that superclasses an existing window class
DLLEXPORT class SafeSuperclass
{
	WNDPROC m_wprocBaseClass;
	TCHAR m_szNewClass[64];
public:
	SafeSuperclass( LPCTSTR pszOldClass, WNDPROC wprocSuperclass, LPCTSTR pszNewClass = NULL );
	~SafeSuperclass();
	LPCTSTR GetClassName() const { return m_szNewClass; }
	LRESULT Chain( HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam ) const;
};



inline
bool WPHSetClassInfo( const WNDCLASS* pstClassInfo )
{
	try
	{
		if( ::RegisterClass( pstClassInfo ) )
			return true;
		if( ::GetLastError() != ERROR_CLASS_ALREADY_EXISTS )
			return false;
		if( ::UnregisterClass( pstClassInfo->lpszClassName, pstClassInfo->hInstance ) )
			return (0 != ::RegisterClass( pstClassInfo ));
		if( ::GetLastError() != ERROR_CLASS_HAS_WINDOWS )
			return false;
	}
	catch( ... )
	{}
	return false;
}


#pragma warning( push )
#pragma warning( disable : 4291 )
inline
WndProcStubBase::WndProcStubBase( WNDPROC wprocTarget, F_CallWindowProc pfCallWindowProcAddress )
:	m_pRouter( NULL )
{
	m_pRouter = (AsmCallWindowProc*)::VirtualAlloc( NULL, sizeof(*m_pRouter), MEM_COMMIT, PAGE_EXECUTE_READWRITE );
	if( !m_pRouter )
		return;
	new( m_pRouter ) AsmCallWindowProc( wprocTarget, pfCallWindowProcAddress );
}
#pragma warning( pop )

inline
WndProcStubBase::~WndProcStubBase()
{
	if( m_pRouter )
		::VirtualFree( m_pRouter, sizeof(*m_pRouter), MEM_RELEASE );
}

inline
bool WndProcStubBase::SetTarget( WNDPROC wprocTarget )
{
	if( !m_pRouter )
		return false;
	m_pRouter->SetWndProc( wprocTarget );
	return true;
}


template<class C>
ThisWndProcStub<C>::ThisWndProcStub( const C* pThis )
	:	m_pPushThisPtrAndJumpToStub( NULL )
{
	m_pPushThisPtrAndJumpToStub =
		(PushThisPtrAndJumpToStub*)VirtualAlloc( NULL, sizeof(*m_pPushThisPtrAndJumpToStub), MEM_COMMIT, PAGE_EXECUTE_READWRITE );
	new( m_pPushThisPtrAndJumpToStub ) PushThisPtrAndJumpToStub( pThis, WindowProcStub );
}

template<class C>
//static
LRESULT CALLBACK ThisWndProcStub<C>::WindowProcStub( C* pThis,
																										 HWND hwnd,
																										 UINT uMsg,
																										 WPARAM wParam,
																										 LPARAM lParam )
{
	return pThis->WindowProc( hwnd, uMsg, wParam, lParam );
}


inline
SafeWndProcHookA::SafeWndProcHookA( HWND hwndTarget, WNDPROC wprocHook )
:	m_pWndProcStub( new WndProcStubA( wprocHook ) )
, m_hwndTarget( hwndTarget )
, m_wprocOriginal( (WNDPROC)::GetWindowLongPtrA( m_hwndTarget, GWLP_WNDPROC ) )
{
	if( m_pWndProcStub )
		::SetWindowLongPtrA( m_hwndTarget, GWLP_WNDPROC, (LONG_PTR)(WNDPROC)*m_pWndProcStub );
}

inline
SafeWndProcHookA::~SafeWndProcHookA()
{
	if( NULL == m_pWndProcStub || IsSafeToRemoveWndProc() )
	{
		::SetWindowLongPtrA( m_hwndTarget, GWLP_WNDPROC, (LONG_PTR)m_wprocOriginal );
		delete m_pWndProcStub;
	}
	else
	{	//leave the stub orphaned in memory
		m_pWndProcStub->SetTarget( m_wprocOriginal );
	}
}

inline
//virtual
bool SafeWndProcHookA::IsSafeToRemoveWndProc()
{
	return (!::IsWindow( m_hwndTarget ) || (WNDPROC)*m_pWndProcStub == (WNDPROC)::GetWindowLongPtrA( m_hwndTarget, GWLP_WNDPROC ));
}

inline
LRESULT SafeWndProcHookA::Chain( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) const
{
	return ::CallWindowProcA( m_wprocOriginal, hwnd, uMsg, wParam, lParam );
}

inline
void SafeWndProcHookA::SetWndProc( WNDPROC wprocTarget )
{
	if( m_pWndProcStub )
		m_pWndProcStub->SetTarget( wprocTarget );
}


inline
SafeWndProcHookW::SafeWndProcHookW( HWND hwndTarget, WNDPROC wprocHook )
:	m_pWndProcStub( new WndProcStubW( wprocHook ) )
, m_hwndTarget( hwndTarget )
, m_wprocOriginal( (WNDPROC)::GetWindowLongPtrW( m_hwndTarget, GWLP_WNDPROC ) )
{
	if( m_pWndProcStub )
		::SetWindowLongPtrW( m_hwndTarget, GWLP_WNDPROC, (LONG_PTR)(WNDPROC)*m_pWndProcStub );
}

inline
SafeWndProcHookW::~SafeWndProcHookW()
{
	if( NULL == m_pWndProcStub || IsSafeToRemoveWndProc() )
	{
		::SetWindowLongPtrW( m_hwndTarget, GWLP_WNDPROC, (LONG_PTR)m_wprocOriginal );
		delete m_pWndProcStub;
	}
	else
	{	//leave the stub orphaned in memory
		m_pWndProcStub->SetTarget( m_wprocOriginal );
	}
}

inline
//virtual
bool SafeWndProcHookW::IsSafeToRemoveWndProc()
{
	return (!::IsWindow( m_hwndTarget ) || (WNDPROC)*m_pWndProcStub == (WNDPROC)::GetWindowLongPtrW( m_hwndTarget, GWLP_WNDPROC ));
}

inline
LRESULT SafeWndProcHookW::Chain( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) const
{
	return ::CallWindowProcW( m_wprocOriginal, hwnd, uMsg, wParam, lParam );
}

inline
void SafeWndProcHookW::SetWndProc( WNDPROC wprocTarget )
{
	if( m_pWndProcStub )
		m_pWndProcStub->SetTarget( wprocTarget );
}


inline
SafeWindowClassHook::SafeWindowClassHook( LPCTSTR pszTargetClass,
																					HMODULE hmodTarget,
																					WNDPROC wprocHook )
:	m_pWndProcStub( new WndProcStub( wprocHook ) )
, m_wprocBaseClass( NULL )
{
	if( m_pWndProcStub &&
		::GetClassInfo( hmodTarget? hmodTarget : ::GetModuleHandle( NULL ),
										pszTargetClass,
										&m_stClassInfo ) )
	{
		if( *m_pWndProcStub != m_stClassInfo.lpfnWndProc )
			m_wprocBaseClass = m_stClassInfo.lpfnWndProc;
		if( *m_pWndProcStub )
			m_stClassInfo.lpfnWndProc = *m_pWndProcStub;
		if( !WPHSetClassInfo( &m_stClassInfo ) )
			m_wprocBaseClass = NULL; //failed
		m_stClassInfo.lpfnWndProc = m_wprocBaseClass; //restore it for the destructor
	}
}

inline
SafeWindowClassHook::~SafeWindowClassHook()
{
	WPHSetClassInfo( &m_stClassInfo );
	if( m_wprocBaseClass && IsSafeToRemoveWndProc() )
		delete m_pWndProcStub;
}

inline
LRESULT SafeWindowClassHook::Chain( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) const
{
	return ::CallWindowProc( m_wprocBaseClass, hwnd, uMsg, wParam, lParam );
}


inline
SafeWindowClassSubclass::SafeWindowClassSubclass( LPCTSTR pszTargetClass, HMODULE hmodTarget )
:	SafeWindowClassHook( pszTargetClass, hmodTarget, m_ThisWndProcStub )
, m_ThisWndProcStub( this )
{
}


inline
SafeSuperclass::SafeSuperclass( LPCTSTR pszOldClass, WNDPROC wprocSuperclass, LPCTSTR pszNewClass /*= NULL*/ )
	:	m_wprocBaseClass( NULL )
{
	WNDCLASS stClassInfo;
	if( ::GetClassInfo( ::GetModuleHandle( NULL ), pszOldClass, &stClassInfo ) )
	{
		if( pszNewClass )
			::lstrcpyn( m_szNewClass, pszNewClass, sizeof(m_szNewClass) - 1 );
		else
		{
			TCHAR szOldClass[64];
			lstrcpyn( szOldClass, pszOldClass, 40 );
			::wsprintf( m_szNewClass, _T("%sSuperclass%p"), szOldClass, this );
		}
		stClassInfo.hInstance = ::GetModuleHandle( NULL );
		stClassInfo.lpszClassName = m_szNewClass;
		if( wprocSuperclass != stClassInfo.lpfnWndProc )
			m_wprocBaseClass = stClassInfo.lpfnWndProc;
		if( wprocSuperclass )
			stClassInfo.lpfnWndProc = wprocSuperclass;
		if( !::RegisterClass( &stClassInfo ) )
			m_szNewClass[0] = _T('\0'); //failed
	}
	else
		m_szNewClass[0] = _T('\0');
}

inline
SafeSuperclass::~SafeSuperclass()
{
	if( m_szNewClass[0] != _T('\0') )
		::UnregisterClass( m_szNewClass, ::GetModuleHandle( NULL ) );
}

inline
LRESULT SafeSuperclass::Chain( HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam ) const
{
	if( m_wprocBaseClass )
		return m_wprocBaseClass( hwnd, iMsg, wParam, lParam );
	else
		return -1;
}


#endif //_WNDPROCHOOK_H_
