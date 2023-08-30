// Copyright 2010 ManuSoft
// https://www.manusoft.com
// All Rights Reserved
//
// Use of this software is governed by the terms of:
// GNU General Public License v2.0
// See LICENSE for details

#include "StdAfx.h"
#include "MemIO.h"
#include "AsmHook.h"
#include "CodeHeap.h"
#include "ThunkHeap.h"

extern "C" void InitHookContext();
extern "C" void ExitHookContext();


static CCodeHeap*& GetCodeHeapPtr()
{
	static CCodeHeap* pHeap = NULL;
	if( !pHeap )
		pHeap = new CCodeHeap;
	return pHeap;
}

static CCodeHeap& GetCodeHeap()
{
	return *GetCodeHeapPtr();
}

static ULONG gctHeapAlloc = 0;

void* CodeHeapAlloc( size_t n )
{
	void* pvMem = GetCodeHeap().Alloc( n );
	if( pvMem )
		++gctHeapAlloc;
	return pvMem;
}

void CodeHeapFree( void* p )
{
	if( !p )
		return;
	CCodeHeap*& pHeap = GetCodeHeapPtr();
	pHeap->Free( p );
	assert( gctHeapAlloc > 0 );
	if( --gctHeapAlloc == 0 )
	{
		delete pHeap;
		pHeap = NULL;
	}
}

AsmPtrJmp* CreateJmpThunk( LPCVOID pvSource, PROC pfThunkTarget )
{
	CThunkHeap* pThunkHeap = CThunkHeap::GetThunkHeap( pvSource );
	assert( pThunkHeap != NULL );
	if( !pThunkHeap )
		return NULL;
	AsmPtrJmp* pvThunk = (AsmPtrJmp*)pThunkHeap->Alloc( sizeof(AsmPtrJmp) );
	assert( pvThunk != NULL );
	if( !pvThunk )
		return NULL;
	DWORD dwOldProt;
	VirtualProtect( pvThunk, sizeof(AsmPtrJmp), PAGE_EXECUTE_READWRITE, &dwOldProt );
	new ( pvThunk ) AsmPtrJmp( pfThunkTarget );
	VirtualProtect( pvThunk, sizeof(AsmPtrJmp), dwOldProt, &dwOldProt );
	return pvThunk;
}

void FreeJmpThunk( AsmPtrJmp* pThunk )
{
	pThunk;
}

#if (_MSC_VER < 1900)
#pragma warning(push)
#pragma warning(disable:4211)
static void* operator new( size_t n ) { return CodeHeapAlloc( n ); }
static void* operator new[]( size_t n ) { return CodeHeapAlloc( n ); }
static void operator delete( void* p ) { CodeHeapFree( p ); }
static void operator delete[]( void* p ) { CodeHeapFree( p ); }
#pragma warning(pop)
#endif


const PROC JmpThunkManager::ThunkTo( LPCVOID pvSrc, const PROC pfTarget )
{
#ifdef _WIN64
	if( mpJmpThunk )
	{
		mpJmpThunk->SetDestination( pfTarget );
		return (const PROC)mpJmpThunk;
	}
	LONG_PTR lOffset = (BYTE*)pfTarget - (BYTE*)pvSrc;
	if( lOffset != (LONG)lOffset )
	{
		mpJmpThunk = CreateJmpThunk( pvSrc, pfTarget );
		assert( mpJmpThunk != NULL );
		lOffset = (BYTE*)mpJmpThunk - (BYTE*)pvSrc;
		assert( lOffset == (LONG)lOffset );
		if( lOffset != (LONG)lOffset )
			throw eBadFormat;
		return (const PROC)mpJmpThunk;
	}
#else
	pvSrc;
#endif //_WIN64
	return pfTarget;
}
#ifdef _WIN64
LONG JmpThunkManager::ThunkToIndirect( LPCVOID pvSrc, const PROC& pfTarget )
{
	if( mpJmpThunk )
	{
		mpJmpThunk->SetDestination( pfTarget );
		LONG_PTR lOffset = (BYTE*)&pfTarget - (BYTE*)pvSrc;
		if( lOffset != (LONG)lOffset )
		{
			FreeJmpThunk( mpJmpThunk );
			mpJmpThunk = CreateJmpThunk( pvSrc, pfTarget );
			assert( mpJmpThunk != NULL );
			lOffset = (BYTE*)mpJmpThunk - (BYTE*)pvSrc;
		}
		assert( lOffset == (LONG)lOffset );
		if( lOffset != (LONG)lOffset )
			throw eBadFormat;
		return (LONG)lOffset;
	}
	LONG_PTR lOffset = (BYTE*)&pfTarget - (BYTE*)pvSrc;
	if( lOffset != (LONG)lOffset )
	{
		mpJmpThunk = CreateJmpThunk( pvSrc, pfTarget );
		assert( mpJmpThunk != NULL );
		lOffset = (BYTE*)mpJmpThunk - (BYTE*)pvSrc;
	}
	assert( lOffset == (LONG)lOffset );
	if( lOffset != (LONG)lOffset )
		throw eBadFormat;
	return (LONG)lOffset;
}
#else
DWORD JmpThunkManager::ThunkToIndirect( LPCVOID pvSrc, const PROC& pfTarget )
{
	pvSrc;
	return (DWORD)&pfTarget;
}
#endif //_WIN64


SafeCall::SafeCall( const PROC pfDestination )
	:	m_ResetSP( sizeof(DWORD_PTR) )
	, m_CallSetup( &m_CallCleanup )
	, m_CallExec( pfDestination )
{
	if( !pfDestination )
		Clear();
}

void SafeCall::Set( const PROC pfDestination )
{
	m_CallExec.SetDestination( pfDestination );
}

void SafeCall::Clear()
{
	FillMemory( this, sizeof(*this), (BYTE)AsmNop() );
}

PROC SafeCall::GetDestination() const
{
	return (PROC)m_CallExec.GetDestination();
}


SafeCallEx::SafeCallEx( const PROC pfDestination, RegisterStateStorage& rRegSave )
:	m_SaveSP2( (DWORD_PTR)&rRegSave + sizeof(rRegSave) - sizeof(DWORD_PTR) )
, m_SwitchToLocalStack1( (DWORD_PTR)&rRegSave + sizeof(rRegSave) - sizeof(DWORD_PTR) )
, m_SwitchToLocalStack2( (DWORD_PTR)&rRegSave )
, m_SaveNewA( sizeof(DWORD_PTR) * 8 )
, m_SafeCall( pfDestination )
, m_RestoreSP3( sizeof(DWORD_PTR) )
{
}

void SafeCallEx::Set( const PROC pfDestination )
{
	m_SafeCall.Set( pfDestination );
}

void SafeCallEx::Clear()
{
	m_SafeCall.Clear();
}

PROC SafeCallEx::GetDestination() const
{
	return m_SafeCall.GetDestination();
}

//static
HookContext* __stdcall HookContext::CleanConstruct( va_list args
																										/*LPVOID pfChain,
																											LPVOID pHookState,
																											LPVOID pfHook,
																											LPVOID pvReturn*/ )
{
	LPVOID pfChain = va_arg(args, LPVOID);
	LPVOID pHookState = va_arg(args, LPVOID);
	LPVOID pfHook = va_arg(args, LPVOID);
	//LPVOID pvReturn = va_arg(args, LPVOID);
	return new HookContext( pfChain, pHookState, pfHook, va_arg(args, LPVOID) );
}

//static
void __stdcall HookContext::CleanDestruct( va_list args /*HookContext* pThis*/ )
{
	delete va_arg(args, HookContext*);
}


StaticCodeProc::StaticCodeProc( LPCVOID pvSource, const ULONG cbOpCodes )
	:	m_JumpToOriginal( LPCVOID(NULL) )
	, m_pvOriginalCode( pvSource? (BYTE*)CodeHeapAlloc(cbOpCodes + sizeof(AsmPtrJmp)) : NULL )
{
	if( !m_pvOriginalCode )
		throw eMemAlloc;
	CopyMemory( m_pvOriginalCode, pvSource, cbOpCodes );

	//If the original code is a call or jmp, we need to correct the relative operand after the move
	//(presumably it is a call or jmp because it was already hooked by someone else)
	if( CompareOpCodeBytes< AsmFarJmp >( m_pvOriginalCode ) )
	{
		m_JumpToOriginal.SetDestination( ((AsmFarJmp*)pvSource)->GetDestination() );
		return;
	}

	__unaligned LPCVOID pvReturnDest = (BYTE*)pvSource + cbOpCodes;
	__unaligned LPVOID pvReturnJmp = (BYTE*)m_pvOriginalCode + cbOpCodes;
	if( CompareOpCodeBytes< AsmFarCall >( m_pvOriginalCode ) )
	{
	#ifndef _WIN64
		((AsmFarCall*)m_pvOriginalCode)->SetDestination( ((AsmFarCall*)pvSource)->GetDestination() );
	#else //_WIN64
		throw eBadFormat;
	#endif //_WIN64
	}
	else if( CompareOpCodeBytes< AsmFarJmp >( m_pvOriginalCode + cbOpCodes - sizeof(AsmFarJmp) ) )
	{ //the last instruction is a far jump, so make it the final AsmPtrJmp
		pvReturnDest = ((AsmFarJmp*)((BYTE*)pvSource + cbOpCodes - sizeof(AsmFarJmp)))->GetDestination();
		pvReturnJmp = (BYTE*)pvReturnJmp - sizeof(AsmFarJmp);
	}

	m_JumpToOriginal.SetDestination( m_pvOriginalCode );
	new (pvReturnJmp) AsmPtrJmp( (PROC)pvReturnDest );
}

StaticCodeProc::~StaticCodeProc()
{
	CodeHeapFree( m_pvOriginalCode );
}


#pragma warning(push)
#pragma warning(disable : 4355)
OneShotStaticCodeProc::OneShotStaticCodeProc( LPCVOID pvSource, const ULONG cbOpCodes )
	:	m_SaveRetAddr( (DWORD_PTR)&m_PushReturnAddress + AsmPushPtr::OffsetOfTargetAddress() )
	, m_JumpToOriginal( LPCVOID(NULL) )
	, m_PushThisPtr( this )
	, m_PushArgSize( sizeof(DWORD_PTR) )
	, m_PushDestructor( (LPVOID)CleanDestruct )
	, m_PushReturnAddress()
	, m_DeleteThis( (LPVOID)PreserveRegsStdCall )
	, m_pvOriginalCode( (BYTE*)CodeHeapAlloc(cbOpCodes + sizeof(AsmPtrJmp)) )
{
	if( !m_pvOriginalCode )
		throw eMemAlloc;
	CopyMemory( m_pvOriginalCode, pvSource, cbOpCodes );

	m_JumpToOriginal.SetDestination( m_pvOriginalCode );
	new ((BYTE*)m_pvOriginalCode + cbOpCodes) AsmPtrJmp( (PROC)&m_PushThisPtr );
}
#pragma warning(pop)

OneShotStaticCodeProc::~OneShotStaticCodeProc()
{
	CodeHeapFree(m_pvOriginalCode);
}

//static
OneShotStaticCodeProc* __stdcall OneShotStaticCodeProc::CleanConstruct( va_list args
																																				/*LPCVOID pvSource,
																																					const ULONG cbOpCodes*/ )
{
	LPCVOID pvSource = va_arg(args,LPCVOID);
	const ULONG cbOpCodes = va_arg(args,const ULONG);
	return new OneShotStaticCodeProc( pvSource, cbOpCodes );
}

//static
void __stdcall OneShotStaticCodeProc::CleanDestruct( va_list args )
{
	delete va_arg(args,OneShotStaticCodeProc*);
}

//static
void __stdcall OneShotBase::CleanDestruct( va_list args )
{
	delete va_arg(args,OneShotBase*);
}


#pragma warning(push)
#pragma warning(disable : 4355)
StructuredCall::StructuredCall( LPCVOID pvReturnAddress,
																const PROC pfPre,
																const PROC pfMain,
																const PROC pfPost )
	:	m_PreCall( pfPre )
	, m_MainCall( pfMain )
	, m_PostCall( pfPost )
	, m_FinalReturnSetup( (DWORD_PTR)pvReturnAddress )
	,	m_ResetSP( sizeof(DWORD_PTR) )
	, m_PushThisPtr( this )
	, m_PushArgSize( sizeof(DWORD_PTR) )
	, m_PushDestructor( (LPVOID)CleanDestruct )
	, m_DeleteThis( (PROC)PreserveRegsStdCall )
{
}
#pragma warning(pop)

//static
StructuredCall* __stdcall StructuredCall::CleanConstruct( va_list args
																													/*LPCVOID pvReturnAddress,
																														const PROC pfPre,
																														const PROC pfMain,
																														const PROC pfPost*/ )
{
	LPCVOID pvReturnAddress = va_arg(args,LPCVOID);
	const PROC pfPre = va_arg(args,const PROC);
	const PROC pfMain = va_arg(args,const PROC);
	const PROC pfPost = va_arg(args,const PROC);
#ifdef _DEBUG
	m_pvLastSource = pvReturnAddress;
#endif
	return new StructuredCall( pvReturnAddress, pfPre, pfMain, pfPost );
}

#ifdef _DEBUG
//static
LPCVOID StructuredCall::m_pvLastSource = NULL;
#endif

//static
void __stdcall StructuredCall::CleanDestruct( va_list args /*StructuredCall* pThis*/ )
{
	delete va_arg(args,StructuredCall*);
}


#pragma warning(push)
#pragma warning(disable : 4355)
StructuredCallEx::StructuredCallEx( LPCVOID pvReturnAddress,
																		RegisterStateStorage& rRegSave,
																		const PROC pfPre,
																		const PROC pfMain,
																		const PROC pfPost )
	:	m_PreCall( pfPre, rRegSave )
	, m_MainCall( pfMain )
	, m_PostCall( pfPost, rRegSave)
	, m_FinalReturnSetup( (DWORD_PTR)pvReturnAddress )
	,	m_ResetSP( sizeof(DWORD_PTR) )
	, m_PushThisPtr( this )
	, m_PushArgSize( sizeof(DWORD_PTR) )
	, m_PushDestructor( (LPVOID)CleanDestruct )
	, m_DeleteThis( (PROC)PreserveRegsStdCall )
{
}
#pragma warning(pop)

//static
StructuredCallEx* __stdcall StructuredCallEx::CleanConstruct( va_list args
																															/*LPCVOID pvReturnAddress,
																																RegisterStateStorage& rRegSave,
																																const PROC pfPre,
																																const PROC pfMain,
																																const PROC pfPost*/ )
{
	LPCVOID pvReturnAddress = va_arg(args,LPCVOID);
	RegisterStateStorage& rRegSave = *va_arg(args,RegisterStateStorage*);
	const PROC pfPre = va_arg(args,const PROC);
	const PROC pfMain = va_arg(args,const PROC);
	const PROC pfPost = va_arg(args,const PROC);
#ifdef _DEBUG
	m_pvLastSource = pvReturnAddress;
#endif
	return new StructuredCallEx( pvReturnAddress, rRegSave, pfPre, pfMain, pfPost );
}

#ifdef _DEBUG
//static
LPCVOID StructuredCallEx::m_pvLastSource = NULL;
#endif

//static
void __stdcall StructuredCallEx::CleanDestruct( va_list args /*StructuredCallEx* pThis*/ )
{
	delete va_arg(args,StructuredCallEx*);
}


CallRouter::CallRouter( const PROC pfPre /*= NULL*/,
												const PROC pfMain /*= NULL*/,
												const PROC pfPost /*= NULL*/ )
	: m_PushPost( (DWORD_PTR)pfPost )
	, m_PushMain( (DWORD_PTR)pfMain )
	, m_PushPre( (DWORD_PTR)pfPre )
	, m_ArgSize( sizeof(DWORD_PTR) * 4 )
	, m_ConstructorProc( (PROC)StructuredCall::CleanConstruct )
	, m_MakeNewStructuredCall( (PROC)PreserveRegsStdCallRet )
{
}


CallRouterEx::CallRouterEx( const PROC pfPre /*= NULL*/,
														const PROC pfMain /*= NULL*/,
														const PROC pfPost /*= NULL*/ )
	: m_PushPost( (DWORD_PTR)pfPost )
	, m_PushMain( (DWORD_PTR)pfMain )
	, m_PushPre( (DWORD_PTR)pfPre )
	, m_PushRegStg( (DWORD_PTR)&m_RegStg )
	, m_ArgSize( sizeof(DWORD_PTR) * 5 )
	, m_ConstructorProc( (PROC)StructuredCallEx::CleanConstruct )
	, m_MakeNewStructuredCall( (PROC)PreserveRegsStdCallRet )
{
}


HookIndirectFarCall::HookIndirectFarCall( AsmFarCallIndirect* pAsmCall,
																					const PROC pfPre /*= NULL*/,
																					const PROC pfPost /*= NULL*/ )
	:	m_pfPre( pfPre )
	, m_pfMain( *(PROC*)pAsmCall->GetDestination() )
	, m_pfPost( pfPost )
	, m_CallRouter( pfPre, *(PROC*)pAsmCall->GetDestination(), pfPost )
	, m_pfCallRouter( (PROC)&m_CallRouter )
	, HookCode( pAsmCall, sizeof(AsmFarCallIndirect) )
{
	PROC& pfHook = m_pfCallRouter;
	MemProtectOverride MemState( pAsmCall, sizeof(AsmFarCallIndirect) );
	pAsmCall->Set( ThunkToIndirect( pAsmCall + 1, pfHook ) );
}


HookStaticCode::HookStaticCode( LPVOID pvSource,
																const ULONG cbOpCodes,
																const PROC pfPre /*= NULL*/,
																const PROC pfPost /*= NULL*/,
																F_Callback pfPreHookCallback /*= NULL*/ )
	: m_PushSize( (DWORD_PTR)cbOpCodes )
	, m_PushSource( (DWORD_PTR)GetSavedCode() )
	, m_ArgSize( sizeof(DWORD_PTR) * 2 )
	, m_ConstructorProc( (PROC)OneShotStaticCodeProc::CleanConstruct )
	, m_MakeNewStaticCodeProc( (PROC)PreserveRegsStdCallRet )
	, m_CallRouter( pfPre,
									*(BYTE*)pvSource == (BYTE)AsmFarCall::_TInstruction::_OpCodes?
										(PROC)((AsmFarCall*)pvSource)->GetDestination() :
										(PROC)&m_SaveEax,
									pfPost )
	, HookCode( pvSource, cbOpCodes )
{
	if( pfPreHookCallback )
	{
		if( !pfPreHookCallback( this ) )
			throw eBadFormat;
	}
	int cbPadding = cbOpCodes - sizeof(AsmFarCall);
	if( cbPadding < 0 )
		throw eBadFormat;

	PROC pfHook = (PROC)&m_CallRouter;
	MemProtectOverride MemState( pvSource, cbOpCodes );
	new (pvSource) AsmFarCall( ThunkTo( (BYTE*)pvSource + sizeof(AsmFarCall), pfHook ) );
	if( cbPadding > 0 )
		FillMemory( (BYTE*)pvSource + sizeof(AsmFarCall), cbPadding, (BYTE)AsmNop() );
}


HookFunctionEntry::HookFunctionEntry( const PROC pfTargetFunction,
																			const ULONG cbOpCodes,
																			const PROC pfWrapperFunction,
																			const FunctionInfo info,
																			LPVOID pHookState /*= NULL*/,
																			F_Callback pfPreHookCallback /*= NULL*/ )
	: mPushHookProc( pfWrapperFunction )
	, mPushHookStatePtr( pHookState )
	, mPushChainPtr( (FARPROC)mChainOriginal )
	, mPushArgSize1( sizeof(DWORD_PTR) * 4 )
	, mPushConstructorProc( &HookContext::CleanConstruct )
	, mSafeCallConstructor( &PreserveRegsStdCallRet )
	, mCallInitHookContext( &InitHookContext )
	, mCallHook( pfWrapperFunction )
#ifdef _WIN64
	, mResetEsp( static_cast<BYTE>(sizeof(DWORD_PTR) * 1) )
#else
	, mResetEsp( static_cast<BYTE>(sizeof(DWORD_PTR) * (info.isCDecl()? 1 : -1)) )
#endif
	, mSaveRetAddress( static_cast<BYTE>(sizeof(DWORD_PTR) * 1) )
	, mRestoreB( static_cast<BYTE>(sizeof(DWORD_PTR) * 5) )
	, mPushArgSize2( sizeof(DWORD_PTR) )
	, mPushDestructor( &HookContext::CleanDestruct )
	, mDeleteHookContext( &PreserveRegsStdCall )
	, mChainOriginal( pfTargetFunction, cbOpCodes )
	, mFI( info )
	, HookCode( pfTargetFunction, cbOpCodes )
{
	if( pfPreHookCallback )
	{
		if( !pfPreHookCallback( this ) )
			throw eBadFormat;
	}
	int cbPadding = cbOpCodes - sizeof(AsmFarJmp);
	if( cbPadding < 0 )
		throw eBadFormat;

#ifdef _WIN64
	PROC pfHook = (PROC)&mPushHookProc;
#else
	PROC pfHook = (info.isThisPtr()? (PROC)&mGetRetAddr : (PROC)&mPushHookProc);
#endif
	MemProtectOverride MemState( pfTargetFunction, cbOpCodes );
	new (pfTargetFunction) AsmFarJmp( ThunkTo( pfTargetFunction, pfHook ) );
	if( cbPadding > 0 )
		FillMemory( (BYTE*)pfTargetFunction + sizeof(AsmFarJmp), cbPadding, (BYTE)AsmNop() );
}


HookFunctionEntry::Unwinder::Unwinder( HookContext*& pHookContext )
	:	OneShotBase( pHookContext->chain() )
	, mGetHookContext( (DWORD_PTR)pHookContext )
	, mUnwindHookStub( &ExitHookContext )
{
	pHookContext->chain() = OneShotBase::operator FARPROC();
	HookFunctionEntry::GetReturnAddress( pHookContext ) = (LPVOID)&mGetHookContext;
}


HookDirectFarCall::HookDirectFarCall( AsmFarCall* pAsmCall,
																			const PROC pfWrapperFunction,
																			const FunctionInfo info,
																			LPVOID pHookState /*= NULL*/,
																			F_Callback pfPreHookCallback /*= NULL*/ )
	: mPushHookProc( pfWrapperFunction )
	, mPushHookStatePtr( pHookState )
	, mPushChainPtr( (FARPROC)pAsmCall->GetDestination() )
	, mPushArgSize1( sizeof(DWORD_PTR) * 4 )
	, mPushConstructorProc( &HookContext::CleanConstruct )
	, mSafeCallConstructor( &PreserveRegsStdCallRet )
	, mCallInitHookContext( &InitHookContext )
	, mCallHook( pfWrapperFunction )
#ifdef _WIN64
	, mResetEsp( static_cast<BYTE>(sizeof(DWORD_PTR) * 1) )
#else
	, mResetEsp( static_cast<BYTE>(sizeof(DWORD_PTR) * (info.isCDecl()? 1 : -1)) )
#endif
	, mSaveRetAddress( static_cast<BYTE>(sizeof(DWORD_PTR) * 1) )
	, mRestoreB( static_cast<BYTE>(sizeof(DWORD_PTR) * 5) )
	, mPushArgSize2( sizeof(DWORD_PTR) )
	, mPushDestructor( &HookContext::CleanDestruct )
	, mDeleteHookContext( &PreserveRegsStdCall )
	, mpfChainOriginal( (FARPROC)pAsmCall->GetDestination() )
	, mFI( info )
	, HookCode( pAsmCall, sizeof(*pAsmCall) )
{
	if( pfPreHookCallback )
	{
		if( !pfPreHookCallback( this ) )
			throw eBadFormat;
	}

#ifdef _WIN64
	PROC pfHook = (PROC)&mPushHookProc;
#else
	PROC pfHook = (info.isThisPtr()? (PROC)&mGetRetAddr : (PROC)&mPushHookProc);
#endif
	MemProtectOverride MemState( pAsmCall, sizeof(*pAsmCall) );
	new (pAsmCall) AsmFarCall( ThunkTo( pAsmCall, pfHook ) );
}


HookDirectFarCall::Unwinder::Unwinder( HookContext*& pHookContext )
	:	OneShotBase( pHookContext->chain() )
	, mGetHookContext( (DWORD_PTR)pHookContext )
	, mUnwindHookStub( &ExitHookContext )
{
	pHookContext->chain() = OneShotBase::operator FARPROC();
	HookFunctionEntry::GetReturnAddress( pHookContext ) = (LPVOID)&mGetHookContext;
}



//static
HookWrapper::WrapperImp* __stdcall HookWrapper::WrapperImp::New( va_list args
																																 /*const PROC pfHookedFunction,
																																	 const PROC pfOnEntry,
																																	 const PROC pfOnExit,
																																	 LPVOID pvReturnAddress*/ )
{
	const PROC pfHookedFunction = va_arg(args,const PROC);
	const PROC pfOnEntry = va_arg(args,const PROC);
	const PROC pfOnExit = va_arg(args,const PROC);
	LPVOID pvReturnAddress = va_arg(args,LPVOID);
	return new HookWrapper::WrapperImp( pfHookedFunction, pfOnEntry, pfOnExit, pvReturnAddress );
}


//static
DWORD_PTR __stdcall HookWrapper::WrapperImp::Delete( va_list args
																										 /*WrapperImp* pThis,
																											 const DWORD_PTR dwResult*/ )
{
	WrapperImp* pThis = va_arg(args,WrapperImp*);
	DWORD_PTR dwFinalResult;
	dwFinalResult = pThis->m_pfOnExit( pThis->m_pWrapper, va_arg(args,DWORD_PTR) );
	delete pThis;
	return dwFinalResult;
}


#ifndef _WIN64
//static
HookWrapper::MemberWrapperImp* __stdcall HookWrapper::MemberWrapperImp::New( va_list args
																																 /*const PROC pfHookedFunction,
																																	 const PROC pfOnEntry,
																																	 const PROC pfOnExit,
																																	 LPVOID pvReturnAddress*/ )
{
	const PROC pfHookedFunction = va_arg(args,const PROC);
	const PROC pfOnEntry = va_arg(args,const PROC);
	const PROC pfOnExit = va_arg(args,const PROC);
	LPVOID pvReturnAddress = va_arg(args,LPVOID);
	return new HookWrapper::MemberWrapperImp( pfHookedFunction, pfOnEntry, pfOnExit, pvReturnAddress );
}


//static
DWORD_PTR __stdcall HookWrapper::MemberWrapperImp::Delete( va_list args
																										 /*MemberWrapperImp* pThis,
																											 const DWORD_PTR dwResult*/ )
{
	MemberWrapperImp* pThis = va_arg(args,MemberWrapperImp*);
	DWORD dwFinalResult;
	dwFinalResult = pThis->m_pfOnExit( pThis->m_pWrapper, va_arg(args,DWORD_PTR) );
	delete pThis;
	return dwFinalResult;
}
#endif //_WIN64


//static
DWORD_PTR NullFunctionWrapper::OnExit( NullFunctionWrapper* pThis, const DWORD_PTR dwResult )
{
	delete pThis;
	return dwResult;
}


WrapCallBase::WrapCallBase( LPVOID pAsmCall, const PROC pfOnEntry, const PROC pfOnExit )
	:	m_pfMain( (PROC)((AsmFarCall*)pAsmCall)->GetDestination() )
	, m_Wrapper( (PROC)((AsmFarCall*)pAsmCall)->GetDestination(), pfOnEntry, pfOnExit )
	, HookCode( pAsmCall, sizeof(AsmFarCall) )
{
	PROC pfHook = (PROC)&m_Wrapper;
	MemProtectOverride MemState( pAsmCall, sizeof(AsmFarCall) );
	((AsmFarCall*)pAsmCall)->SetDestination( ThunkTo( (BYTE*)((AsmFarCall*)pAsmCall + 1), pfHook ) );
}


WrapCallTableEntryBase::WrapCallTableEntryBase( const PROC* pEntry,
																								const PROC pfOnEntry,
																								const PROC pfOnExit,
																								bool bMemberFunction /*= false*/ )
	:	m_pfMain( *pEntry )
	, m_Wrapper( *pEntry, pfOnEntry, pfOnExit, bMemberFunction )
	, HookCode( (LPVOID)pEntry, sizeof(PROC*) )
{
	Poke( (LPVOID)pEntry, (PROC)&m_Wrapper );
}


WrapFunctionEntryBase::WrapFunctionEntryBase( const PROC pfTargetFunction,
																							const ULONG cbOpCodes,
																							const PROC pfOnEntry,
																							const PROC pfOnExit,
																							F_Callback pfPreHookCallback /*= NULL*/,
																							bool bMemberFunction /*= false*/ )
	: m_Wrapper( (PROC)&m_OriginalFunction, pfOnEntry, pfOnExit, bMemberFunction )
	, m_OriginalFunction( pfTargetFunction, cbOpCodes )
	, HookCode( pfTargetFunction, cbOpCodes )
{
	if( pfPreHookCallback )
	{
		if( !pfPreHookCallback( this ) )
			throw eBadFormat;
	}
	int cbPadding = cbOpCodes - sizeof(AsmFarJmp);
	if( cbPadding < 0 )
		throw eBadFormat;

	PROC pfHook = (PROC)&m_Wrapper;
	MemProtectOverride MemState( pfTargetFunction, cbOpCodes );
	new (pfTargetFunction) AsmFarJmp( ThunkTo( (BYTE*)pfTargetFunction + sizeof(AsmFarJmp), pfHook ) );
	if( cbPadding > 0 )
		FillMemory( (BYTE*)pfTargetFunction + sizeof(AsmFarJmp), cbPadding, (BYTE)AsmNop() );
}


//static
HookCallTable::HookCallTable( const PROC* pEntry,
															const PROC pfPre /*= NULL*/,
															const PROC pfPost /*= NULL*/ )
	:	m_pfPre( pfPre )
	, m_pfMain( *pEntry )
	, m_pfPost( pfPost )
	, m_CallRouter( pfPre, *pEntry, pfPost )
	, HookCode( (LPVOID)pEntry, sizeof(PROC*) )
{
	Poke( (LPVOID)pEntry, (PROC)&m_CallRouter );
}


HookJumpTable::HookJumpTable( PROC* pEntry,
															const PROC pfHook /*= NULL*/ )
	:	m_HookExec( pfHook )
	, m_OriginalCodeJmp( *pEntry )
	, HookCode( pEntry, sizeof(PROC*) )
{
	Poke( pEntry, (PROC)&m_HookExec );
}


PtrTablePatch::PtrTablePatch( LPVOID const* rpvTable, ULONG ctEntries )
	:	m_rpvTable( rpvTable )
	, m_ctEntries( ctEntries )
	, HookCode( (LPVOID)rpvTable, ctEntries * sizeof(LPVOID) )
{}

bool PtrTablePatch::Restore( int idxTarget )
{
	return Set( idxTarget, ((LPVOID*)GetSavedCode())[idxTarget] );
}

bool PtrTablePatch::RestoreAll()
{
	RestoreSavedCode();
	return true;
}

bool PtrTablePatch::Set( int idxTarget, const void* const pvNew )
{
	if( idxTarget < 0 || (ULONG)idxTarget >= m_ctEntries )
		return false;
	Poke( (LPVOID)&m_rpvTable[idxTarget], pvNew );
	return true;
}


PatchCode::PatchCode( LPVOID pvTarget,
											const ULONG cbOpCodes,
											const PROC pfHook /*= NULL*/ )
	:	m_PushReturn( ((BYTE*)pvTarget + cbOpCodes) )
	, m_PatchExec( (LPVOID)pfHook )
	, HookCode( pvTarget, cbOpCodes )
{
	if( cbOpCodes < sizeof(AsmFarJmp) )
		throw eBadFormat;
	PROC pfHookStub = (PROC)&m_PushReturn;
	MemProtectOverride MemState( pvTarget, sizeof(AsmFarJmp) );
	new (pvTarget) AsmFarJmp( ThunkTo( (BYTE*)pvTarget + sizeof(AsmFarJmp), pfHookStub ) );
}


PatchIndirectFarCall::PatchIndirectFarCall( LPVOID pvTarget,
																						const PROC* ppfHook /*= NULL*/ )
	:	HookCode( pvTarget, sizeof(AsmFarCallIndirect) )
{
	if( ppfHook )
	{
		MemProtectOverride MemState( pvTarget, sizeof(AsmFarCallIndirect) );
		new (pvTarget) AsmFarCallIndirect( ppfHook );
	}
}


NopPatch::NopPatch( LPVOID pvTarget, ULONG cbOpCodes )
	:	HookCode( pvTarget, cbOpCodes )
{
	MemProtectOverride MemState( pvTarget, cbOpCodes );
	FillMemory( pvTarget, cbOpCodes, (BYTE)AsmNop() );
}
