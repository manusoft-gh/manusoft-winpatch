// Copyright 2010 ManuSoft
// https://www.manusoft.com
// All Rights Reserved
//
// Use of this software is governed by the terms of:
// GNU General Public License v2.0
// See LICENSE for details

#pragma once

#ifndef _ASMHOOK_H_
#define _ASMHOOK_H_

#include "Patch.h"
#include "Asm.h"


#ifndef DLLEXPORT
#define DLLEXPORT
#endif

#ifndef assert
#define assert(_Expression)     ((void)0)
#endif //asert

//for use by code heap (executable memory) allocators
void* CodeHeapAlloc( size_t n );
void CodeHeapFree( void* p );


AsmPtrJmp* CreateJmpThunk( LPCVOID pvSource, PROC pfThunkTarget );
void FreeJmpThunk( AsmPtrJmp* pThunk );


//functions defined in assembly language
typedef void (__stdcall *F_StdCallTargetFunction) ( va_list args );
extern "C" void __stdcall PreserveRegsStdCall( F_StdCallTargetFunction pfTarget, DWORD_PTR cbArgs, ... );
typedef DWORD_PTR (__stdcall *F_StdCallTargetFunctionRet) ( va_list args );
extern "C" DWORD_PTR __stdcall PreserveRegsStdCallRet( F_StdCallTargetFunctionRet pfTarget, DWORD_PTR cbArgs, ... );
typedef void (__cdecl *F_CDeclTargetFunction) ( va_list args );
extern "C" void __stdcall PreserveRegsCDecl( F_CDeclTargetFunction pfTarget, DWORD_PTR cbArgs, ... );
typedef DWORD_PTR (__cdecl *F_CDeclTargetFunctionRet) ( va_list args );
extern "C" DWORD_PTR __stdcall PreserveRegsCDeclRet( F_CDeclTargetFunctionRet pfTarget, DWORD_PTR cbArgs, ... );


#ifdef _WIN64
extern "C" void InsertArgs();
extern "C" void InsertArgsThis();
#endif


class FunctionInfo
{
public:
	enum CConv { StdCall = 0,     //__stdcall (callee removes arguments from stack)
							 CDecl = 1,       //__cdecl (caller removes arguments from stack)
							 FastCall = 2,    //__fastcall (first two arguments in registers)
							 ThisCall = 3,    //__thiscall (like stdcall, but this ptr in ecx register)
						 }; //calling convention of hooked function
	enum HType { Naked = 0,            //enter hook with stack frame identical to hooked function
							 ChainArg = 1,         //insert address of original function as first argument to hook function
							 ThisPtr = 3,          //insert "this" pointer as first argument to hook function, ChainArg as second
							 HookState = 5,        //insert pointer to hook object as first argument to hook function, ChainArg as third
							 HookStateThisPtr = 7, //insert pointer to hook object as first argument to hook function, "this" pointer as second, ChainArg as third
						 }; //hook prototype style
private:
	unsigned int ac : 8; //argument count
	CConv cc : 8;
	HType ht : 8;
public:
	FunctionInfo() : ac( 0 ), cc( StdCall ), ht( Naked ) {}
	FunctionInfo( unsigned int ctArgs, CConv eCallingConvention, HType eHookType )
		: ac( ctArgs ), cc( eCallingConvention ), ht( eHookType )
		{}
	bool isStdCall() const { return (cc == StdCall); }
	bool isCDecl() const { return (cc == CDecl); }
	bool isFastCall() const { return (cc == FastCall); }
	bool isMember() const { return (cc == ThisCall); }
	bool isNaked() const { return (ht == Naked); }
	bool isChainArg() const { return ((ht & ChainArg) == ChainArg); }
	bool isThisPtr() const { return ((ht & ThisPtr) == ThisPtr); }
	bool isHookState() const { return ((ht & HookState) == HookState); }
	unsigned int argCount() const { return ac; }
};

template< unsigned int _cc, unsigned int _ht >
struct TFunctionInfo : public FunctionInfo
{
	TFunctionInfo( unsigned int ctArgs ) : FunctionInfo( ctArgs, (CConv)_cc, (HType)_ht ) {}
};

typedef TFunctionInfo< FunctionInfo::StdCall, FunctionInfo::HookState > FIStdCallHookState;
typedef TFunctionInfo< FunctionInfo::CDecl, FunctionInfo::HookState > FICDeclHookState;
typedef TFunctionInfo< FunctionInfo::ThisCall, FunctionInfo::ThisPtr > FIThisCall;
typedef TFunctionInfo< FunctionInfo::ThisCall, FunctionInfo::HookStateThisPtr > FIThisCallHookState;

#pragma pack(push)
#pragma pack(1)


class DLLEXPORT CodeAllocator
{
public:
	static void* operator new( size_t n ) { return CodeHeapAlloc( n ); }
	static void* operator new[]( size_t n ) { return CodeHeapAlloc( n ); }
	static void operator delete( void* p ) { CodeHeapFree( p ); }
	static void operator delete[]( void* p ) { CodeHeapFree( p ); }
	static void* operator new( size_t, void* _Where ) { return (_Where); }
	static void* operator new[]( size_t, void* _Where ) { return (_Where); }
	static void operator delete( void*, void* ) {}
	static void operator delete[]( void*, void* ) {}
};


class DLLEXPORT JmpThunkManager
{
#ifdef _WIN64
	AsmPtrJmp* mpJmpThunk;
#endif //_WIN64
protected:
	const PROC ThunkTo( LPCVOID pvSrc, const PROC pfTarget );
#ifdef _WIN64
	LONG ThunkToIndirect( LPCVOID pvSrc, const PROC& pfTarget );
#else
	DWORD ThunkToIndirect( LPCVOID pvSrc, const PROC& pfTarget );
#endif //_WIN64
public:
	JmpThunkManager()
	#ifdef _WIN64
		: mpJmpThunk( NULL )
	#endif //_WIN64
		{
		}
protected:
	virtual ~JmpThunkManager()
		{
		#ifdef _WIN64
			FreeJmpThunk( mpJmpThunk );
		#endif //_WIN64
		}
};


class DLLEXPORT HookCode : public CodeAllocator, public JmpThunkManager
{
	PatchBackup m_patchSave;
public:
	HookCode( LPVOID pvTarget, ULONG cbOpCodes )
		:	m_patchSave( pvTarget, cbOpCodes )
		{}
	virtual	~HookCode() {}
	LPVOID GetSavedCode() const { return m_patchSave.GetBackup(); }
	LPVOID GetTarget() const { return m_patchSave.GetTarget(); }
	void RestoreSavedCode() { m_patchSave.Unpatch(); }
};


// SafeCall ensures that the called hook function returns to the following instruction
class SafeCall : public CodeAllocator
{
	AsmAddSPX m_ResetSP;
	AsmPushPtr m_CallSetup;
	AsmPtrJmp m_CallExec;
	AsmPushA m_CallCleanup;
public:
	SafeCall( const PROC pfDestination );
	void Set( const PROC pfDestination );
	void Clear();
	PROC GetDestination() const;
};


struct RegisterStateStorage
{
	DWORD_PTR rSave[10];
};

class HookContext : public CodeAllocator
{
	LPVOID mpvReturn;
	// register storage
	DWORD_PTR regBP;
	DWORD_PTR regSI;
	DWORD_PTR regDI;
	DWORD_PTR regA;
	DWORD_PTR regB;
	DWORD_PTR regC;
	DWORD_PTR regD;
	DWORD_PTR regFlags;
#ifdef _WIN64
	DWORD_PTR regR8;
	DWORD_PTR regR9;
	DWORD_PTR regR10;
	DWORD_PTR regR11;
	DWORD_PTR regR12;
	DWORD_PTR regR13;
#endif
	LPVOID mpvStackPtr;
	LPVOID mpfChain; //address to jump to for executing original code
	LPVOID mpHookState; //pointer to hook object
	LPVOID mpfHook;
public:
	HookContext( LPVOID pfChain, LPVOID pHookState, LPVOID pfHook, LPVOID& pvReturn )
		: mpfChain( pfChain )
		, mpHookState( pHookState )
		, mpfHook( pfHook )
		, mpvReturn( pvReturn )
		, mpvStackPtr( &pvReturn )
		{}
	~HookContext() {}
	template< typename TPtr > TPtr* thisPtr() { return reinterpret_cast<TPtr*>(regC); }
	LPVOID& returnAddress() { return mpvReturn; }
	LPVOID& chain() { return mpfChain; }
	LPVOID& stackPtr() { return mpvStackPtr; }
	template< typename TPtr > TPtr* hookState() { return static_cast<TPtr*>(mpHookState); }
	//clean construction/destruction
	static HookContext* __stdcall CleanConstruct( va_list args
																								/*LPVOID pfChain,
																									LPVOID pHookState,
																									LPVOID pfHook,
																									LPVOID pvReturn*/ );
	static void __stdcall CleanDestruct( va_list args /*HookContext* pThis*/ );
};


class SafeCallEx : public CodeAllocator
{
	AsmPushA m_SaveA1;
	AsmMovASP m_SaveSP1;
	AsmMovIXA m_SaveSP2;
	AsmMovSPX m_SwitchToLocalStack1;
#ifdef _WIN64
	AsmPushRax m_SaveRax;
	AsmPushRbx m_SaveRbx;
	AsmPushRcx m_SaveRcx;
	AsmPushRdx m_SaveRdx;
	AsmPushR8 m_SaveR8;
	AsmPushR9 m_SaveR9;
	AsmPushR10 m_SaveR10;
	AsmPushR11 m_SaveR11;
#else
	AsmPushAD m_SaveRegs;
#endif
	AsmPushF m_SaveFlags;
	AsmMovSPA m_RestoreSP1;
	AsmPopA m_RestoreA1;
	SafeCall m_SafeCall;
	AsmMovSPX m_SwitchToLocalStack2;
	AsmMovISPXA m_SaveNewA;
	AsmPopF m_RestoreFlags;
#ifdef _WIN64
	AsmPopR11 m_RestoreR11;
	AsmPopR10 m_RestoreR10;
	AsmPopR9 m_RestoreR9;
	AsmPopR8 m_RestoreR8;
	AsmPopRdx m_RestoreRdx;
	AsmPopRcx m_RestoreRcx;
	AsmPopRbx m_RestoreRbx;
	AsmPopRax m_RestoreRax;
#else
	AsmPopAD m_RestoreRegs;
#endif
	AsmPopSP m_RestoreSP2;
	AsmAddSPX m_RestoreSP3;
public:
	SafeCallEx( const PROC pfDestination, RegisterStateStorage& rRegSave );
	void Set( const PROC pfDestination );
	void Clear();
	PROC GetDestination() const;
};


class StaticCodeProc : public CodeAllocator
{
	AsmPtrJmp m_JumpToOriginal;
	BYTE* m_pvOriginalCode;
public:
	StaticCodeProc( LPCVOID pvSource, const ULONG cbOpCodes );
	~StaticCodeProc();
	operator FARPROC() { return (FARPROC)&m_JumpToOriginal; }
	LPVOID GetOriginalCode() const { return m_pvOriginalCode; }
};


class OneShotStaticCodeProc : public CodeAllocator
{
	AsmXchgAISP m_SaveAGetRetAddr;
	AsmMovIXA m_SaveRetAddr;
	AsmPopA m_RestoreA;
	AsmPtrJmp m_JumpToOriginal;
	AsmPushPtr m_PushThisPtr;
	AsmPushPtr m_PushArgSize;
	AsmPushPtr m_PushDestructor;
	AsmPushPtr m_PushReturnAddress;
	AsmPtrJmp m_DeleteThis;
	BYTE* m_pvOriginalCode;
	OneShotStaticCodeProc( LPCVOID pvSource, const ULONG cbOpCodes );
	~OneShotStaticCodeProc();
public:
	static OneShotStaticCodeProc* __stdcall CleanConstruct( va_list args
																												 /*LPCVOID pvSource,
																													 const ULONG cbOpCodes*/ );
private:
	static void __stdcall CleanDestruct( va_list args );
};


class OneShotBase : public CodeAllocator
{
	AsmPushPtr m_PushThisPtr;
	AsmPushPtr m_PushArgSize;
	AsmPushPtr m_PushDestructor;
	AsmPushPtr m_PushReturnAddress;
	AsmPtrJmp m_DeleteThis;
public:
#pragma warning(push)
#pragma warning(disable : 4355)
	OneShotBase( LPCVOID pvReturnAddress )
		:	m_PushThisPtr( this )
		, m_PushArgSize( sizeof(DWORD_PTR) )
		, m_PushDestructor( (LPVOID)CleanDestruct )
		, m_PushReturnAddress( (DWORD_PTR)pvReturnAddress )
		, m_DeleteThis( (LPVOID)PreserveRegsStdCall )
		{}
#pragma warning(pop)
	virtual ~OneShotBase() {}
	operator FARPROC() const { return (FARPROC)&m_PushThisPtr; }
private:
	static void __stdcall CleanDestruct( va_list args );
};


class StructuredCall : public CodeAllocator
{
	SafeCall m_PreCall;
	SafeCall m_MainCall;
	SafeCall m_PostCall;
	AsmAddSPX m_ResetSP;
	AsmPushPtr m_PushThisPtr;
	AsmPushPtr m_PushArgSize;
	AsmPushPtr m_PushDestructor;
	AsmPushA m_SaveEax;
	AsmMovAX m_FinalReturnSetup;
	AsmXchgAISP m_RestoreEax;
	AsmPtrJmp m_DeleteThis;
	StructuredCall( LPCVOID pvReturnAddress,
									const PROC pfPre = NULL,
									const PROC pfMain = NULL,
									const PROC pfPost = NULL );
	~StructuredCall() {}
public:
	static StructuredCall* __stdcall CleanConstruct( va_list args
																									 /*LPCVOID pvReturnAddress,
																										 const PROC pfPre = NULL,
																										 const PROC pfMain = NULL,
																										 const PROC pfPost = NULL*/ );
#ifdef _DEBUG
	static LPCVOID m_pvLastSource;
#endif
private:
	static void __stdcall CleanDestruct( va_list args /*StructuredCall* pThis*/ );
};


class StructuredCallEx : public CodeAllocator
{
	SafeCallEx m_PreCall;
	SafeCall m_MainCall;
	SafeCallEx m_PostCall;
	AsmAddSPX m_ResetSP;
	AsmPushPtr m_PushThisPtr;
	AsmPushPtr m_PushArgSize;
	AsmPushPtr m_PushDestructor;
	AsmPushA m_SaveEax;
	AsmMovAX m_FinalReturnSetup;
	AsmXchgAISP m_RestoreEax;
	AsmPtrJmp m_DeleteThis;
	StructuredCallEx( LPCVOID pvReturnAddress,
										RegisterStateStorage& rRegSave,
										const PROC pfPre = NULL,
										const PROC pfMain = NULL,
										const PROC pfPost = NULL );
	~StructuredCallEx() {}
public:
	static StructuredCallEx* __stdcall CleanConstruct( va_list args
																										 /*LPCVOID pvReturnAddress,
																											 RegisterStateStorage& rRegSave,
																											 const PROC pfPre = NULL,
																											 const PROC pfMain = NULL,
																											 const PROC pfPost = NULL*/ );
#ifdef _DEBUG
	static LPCVOID m_pvLastSource;
#endif
private:
	static void __stdcall CleanDestruct( va_list args /*StructuredCallEx* pThis*/ );
};


class CallRouter : public CodeAllocator
{
	AsmXchgAISP m_SaveEaxGetRetAddr;
	AsmPushPtr m_PushPost;
	AsmPushPtr m_PushMain;
	AsmPushPtr m_PushPre;
	AsmPushA m_PushRetAddr;
	AsmPushPtr m_ArgSize;
	AsmPushPtr m_ConstructorProc;
	AsmPtrCall m_MakeNewStructuredCall;
	AsmXchgAISP m_RestoreEax;
	AsmJmpISPX m_GoToCall;
public:
	CallRouter( const PROC pfPre = NULL, const PROC pfMain = NULL, const PROC pfPost = NULL );
	~CallRouter() {}
};


class CallRouterEx : public CodeAllocator
{
	AsmXchgAISP m_SaveEaxGetRetAddr;
	AsmPushPtr m_PushPost;
	AsmPushPtr m_PushMain;
	AsmPushPtr m_PushPre;
	AsmPushPtr m_PushRegStg;
	AsmPushA m_PushRetAddr;
	AsmPushPtr m_ArgSize;
	AsmPushPtr m_ConstructorProc;
	AsmPtrCall m_MakeNewStructuredCall;
	AsmXchgAISP m_RestoreEax;
	AsmJmpISPX m_GoToCall;
	RegisterStateStorage m_RegStg;
public:
	CallRouterEx( const PROC pfPre = NULL, const PROC pfMain = NULL, const PROC pfPost = NULL );
	~CallRouterEx() {}
};



template<class TPtr>
class HookStubInsertArgs : public CodeAllocator
{
#ifdef _WIN64
	AsmPushPtr EntryPoint;
	AsmPushPtr HookAddr;
	AsmPushPtr PushChain;
	AsmPushPtr PushPtr;
	AsmPtrJmp InsertPtrArgs;
#else
	AsmPopA EntryPoint;
	AsmPushC PushThisPtr;
	AsmPushPtr PushChain;
	AsmPushPtr PushPtr;
	AsmPushA PushRetAddr;
	AsmPtrJmp HookAddr;
#endif
public:
	HookStubInsertArgs<TPtr>( const PROC pfTargetFunction,
														const PROC pfHook,
														const FunctionInfo info,
														const TPtr* pPtr = NULL )
		: PushPtr( (DWORD_PTR)pPtr ), HookAddr( pfHook ), PushChain( pfTargetFunction )
	#ifdef _WIN64
		, EntryPoint( (DWORD_PTR)info.argCount() ), InsertPtrArgs( info.isThisPtr()? &InsertArgsThis : &InsertArgs )
	#endif
		{
		#ifndef _WIN64
			if( !info.isThisPtr() )
				FillMemory( &PushThisPtr, sizeof(PushThisPtr), (BYTE)AsmNop() );
		#endif
		}
	~HookStubInsertArgs() {}
	operator FARPROC() const { return (FARPROC)&EntryPoint; }
};

template<class Tx>
class HookCall : public HookCode
{
protected:
	PROC m_pfPre;
	PROC m_pfMain;
	PROC m_pfPost;
	CallRouter m_CallRouter;
public:
	HookCall( Tx* pAsmCall,
						const PROC pfPre = NULL,
						const PROC pfPost = NULL );
	~HookCall() {}
	PROC GetOriginalDestination() const { return m_pfMain; }
};

template<class Tx>
HookCall<Tx>::HookCall( Tx* pAsmCall,
												const PROC pfPre /*= NULL*/,
												const PROC pfPost /*= NULL*/ )
:	m_pfPre( pfPre ),
	m_pfMain( (PROC)pAsmCall->GetDestination() ),
	m_pfPost( pfPost ),
	m_CallRouter( pfPre, (PROC)pAsmCall->GetDestination(), pfPost ),
	HookCode( pAsmCall, sizeof(Tx) )
{
	PROC pfHook = (PROC)&m_CallRouter;
	MemProtectOverride MemState( pAsmCall, sizeof(Tx) );
	pAsmCall->SetDestination( ThunkTo( pAsmCall + 1, (PROC)&m_CallRouter ) );
}

typedef HookCall<AsmFarCall> HookDirectFarCallNaked;


class DLLEXPORT HookIndirectFarCall : public HookCode
{
protected:
	PROC m_pfPre;
	PROC m_pfMain;
	PROC m_pfPost;
	CallRouter m_CallRouter;
	PROC m_pfCallRouter;
public:
	HookIndirectFarCall( AsmFarCallIndirect* pAsmCall,
											 const PROC pfPre = NULL,
											 const PROC pfPost = NULL );
	~HookIndirectFarCall() {}
	PROC GetOriginalDestination() const { return m_pfMain; }
};


class DLLEXPORT HookStaticCode : public HookCode
{
private:
	AsmPushA m_SaveEax;
	AsmPushPtr m_PushSize;
	AsmPushPtr m_PushSource;
	AsmPushPtr m_ArgSize;
	AsmPushPtr m_ConstructorProc;
	AsmPtrCall m_MakeNewStaticCodeProc;
	AsmXchgAISP m_RestoreEax;
	AsmRet m_GoToCall;
	CallRouterEx m_CallRouter;
public:
	typedef bool (*F_Callback)( HookStaticCode* pThis );
	HookStaticCode( LPVOID pvSource,
									const ULONG cbOpCodes,
									const PROC pfPre = NULL,
									const PROC pfPost = NULL,
									F_Callback pfPreHookCallback = NULL );
	~HookStaticCode() {}
};


template< class TPtr = HookStaticCode >
class HookStaticCodePtr : public HookStaticCode
{
private:
	HookStubInsertArgs<TPtr> m_stubPtrPre;
	HookStubInsertArgs<TPtr> m_stubPtrPost;
public:
	HookStaticCodePtr<TPtr>( const TPtr* pArg,
													 LPCVOID pvSource,
													 const ULONG cbOpCodes,
													 const PROC pfPre = NULL,
													 const PROC pfPost = NULL,
													 F_Callback pfPreHookCallback = NULL )
		: HookStaticCode( pvSource,
											cbOpCodes,
											pfPre? m_stubPtrPre : NULL,
											pfPost? m_stubPtrPost : NULL,
											pfPreHookCallback ),
			m_stubPtrPre( pfPre, false, pArg ),
			m_stubPtrPost( pfPost, false, pArg )
		{}
	HookStaticCodePtr<TPtr>( LPCVOID pvSource,
													 const ULONG cbOpCodes,
													 const PROC pfPre = NULL,
													 const PROC pfPost = NULL,
													 F_Callback pfPreHookCallback = NULL )
		: HookStaticCode( pvSource,
											cbOpCodes,
											pfPre? m_stubPtrPre : NULL,
											pfPost? m_stubPtrPost : NULL,
											pfPreHookCallback ),
			m_stubPtrPre( pfPre, false, static_cast<TPtr*>(this) ),
			m_stubPtrPost( pfPost, false, static_cast<TPtr*>(this) )
		{}
	~HookStaticCodePtr<TPtr>() {}
};


class DLLEXPORT HookFunctionEntry : public HookCode
{
	FunctionInfo mFI;
private:
#ifndef _WIN64
	AsmPopA mGetRetAddr;
	AsmPushC mInsertThisPtr;
	AsmPushA mRestoreRetAddr;
#endif
	AsmPushPtr mPushHookProc;
	AsmPushPtr mPushHookStatePtr;
	AsmPushPtr mPushChainPtr;
	AsmPushPtr mPushArgSize1;
	AsmPushPtr mPushConstructorProc;
	AsmPtrCall mSafeCallConstructor;
	AsmPtrCall mCallInitHookContext;
	AsmPtrCall mCallHook;
	AsmAddSPX mResetEsp; //point SP to address expected by caller upon return
	AsmPushA mSaveRetVal;
	AsmMovAIB mGetReturnAddress;
	AsmMovISPXA mSaveRetAddress;
	AsmPushB mSaveHookContext;
	AsmMovBIBX mRestoreB;
	AsmPushPtr mPushArgSize2;
	AsmPushPtr mPushDestructor;
	AsmPtrCall mDeleteHookContext;
	AsmPopA mRestoreRetVal;
	AsmRet mRet;

	StaticCodeProc mChainOriginal;

public:
	typedef bool (*F_Callback)( HookFunctionEntry* pThis );

	HookFunctionEntry( const PROC pfTargetFunction,
										 const ULONG cbOpCodes,
										 const PROC pfWrapperFunction,
										 const FunctionInfo info,
										 LPVOID pHookState = NULL,
										 F_Callback pfPreHookCallback = NULL );
	~HookFunctionEntry() {}
	virtual FARPROC Unwind( HookContext*& pHookContext ) //Setup to unwind the stack and chain to the original function
		{
			Unwinder* pUnwinder = new Unwinder( pHookContext );
		#ifndef _WIN64
			if( mFI.isThisPtr() )
				(DWORD_PTR&)pHookContext->stackPtr() += sizeof(DWORD_PTR);
		#endif
			return pUnwinder->operator FARPROC();
		}
	static LPVOID& GetReturnAddress( HookContext*& pHookContext )
		{
			return *(LPVOID*)((DWORD_PTR*)&pHookContext - 1);
		}
	FARPROC GetOriginalCode() const { return (FARPROC)mChainOriginal.GetOriginalCode(); }
private:
	class Unwinder : public OneShotBase
	{
		AsmMovAX mGetHookContext;
		AsmPtrJmp mUnwindHookStub;
	public:
		Unwinder( HookContext*& pHookContext );
		~Unwinder() {};
		operator FARPROC() const { return (FARPROC)&mGetHookContext; }
	};
};


template< class TPtr = HookFunctionEntry >
class HookFunctionEntryPtr : public HookFunctionEntry
{
public:
	HookFunctionEntryPtr<TPtr>( const TPtr* pArg,
															const PROC pfTargetFunction,
															const ULONG cbOpCodes,
															const PROC pfWrapperFunction,
															const FunctionInfo info,
															F_Callback pfPreHookCallback = NULL )
		: HookFunctionEntry( pfTargetFunction, cbOpCodes, pfWrapperFunction, info, const_cast<TPtr*>(pArg), pfPreHookCallback )
		{}
	HookFunctionEntryPtr<TPtr>( const PROC pfTargetFunction,
															const ULONG cbOpCodes,
															const PROC pfWrapperFunction,
															const FunctionInfo info,
															F_Callback pfPreHookCallback = NULL )
		: HookFunctionEntry( pfTargetFunction, cbOpCodes, pfWrapperFunction, info, const_cast<HookFunctionEntryPtr<TPtr>*>(this), pfPreHookCallback )
		{}
	~HookFunctionEntryPtr<TPtr>() {}
};
typedef HookFunctionEntryPtr<> HookFunctionEntryP;


class DLLEXPORT HookDirectFarCall : public HookCode
{
	FunctionInfo mFI;
private:
#ifndef _WIN64
	AsmPopA mGetRetAddr;
	AsmPushC mInsertThisPtr;
	AsmPushA mRestoreRetAddr;
#endif
	AsmPushPtr mPushHookProc;
	AsmPushPtr mPushHookStatePtr;
	AsmPushPtr mPushChainPtr;
	AsmPushPtr mPushArgSize1;
	AsmPushPtr mPushConstructorProc;
	AsmPtrCall mSafeCallConstructor;
	AsmPtrCall mCallInitHookContext;
	AsmPtrCall mCallHook;
	AsmAddSPX mResetEsp; //point SP to address expected by caller upon return
	AsmPushA mSaveRetVal;
	AsmMovAIB mGetReturnAddress;
	AsmMovISPXA mSaveRetAddress;
	AsmPushB mSaveHookContext;
	AsmMovBIBX mRestoreB;
	AsmPushPtr mPushArgSize2;
	AsmPushPtr mPushDestructor;
	AsmPtrCall mDeleteHookContext;
	AsmPopA mRestoreRetVal;
	AsmRet mRet;

	FARPROC mpfChainOriginal;

public:
	typedef bool (*F_Callback)( HookDirectFarCall* pThis );

	HookDirectFarCall( AsmFarCall* pAsmCall,
										 const PROC pfWrapperFunction,
										 const FunctionInfo info,
										 LPVOID pHookState = NULL,
										 F_Callback pfPreHookCallback = NULL );
	~HookDirectFarCall() {}
	virtual FARPROC Unwind( HookContext*& pHookContext ) //Setup to unwind the stack and chain to the original function
		{
			Unwinder* pUnwinder = new Unwinder( pHookContext );
		#ifndef _WIN64
			if( mFI.isThisPtr() )
				(DWORD_PTR&)pHookContext->stackPtr() += sizeof(DWORD_PTR);
		#endif
			return pUnwinder->operator FARPROC();
		}
	static LPVOID& GetReturnAddress( HookContext*& pHookContext )
		{
			return *(LPVOID*)((DWORD_PTR*)&pHookContext - 1);
		}
	FARPROC GetOriginalCode() const { return mpfChainOriginal; }
private:
	class Unwinder : public OneShotBase
	{
		AsmMovAX mGetHookContext;
		AsmPtrJmp mUnwindHookStub;
	public:
		Unwinder( HookContext*& pHookContext );
		~Unwinder() {};
		operator FARPROC() const { return (FARPROC)&mGetHookContext; }
	};
};


template< class TPtr = HookDirectFarCall >
class HookDirectFarCallPtr : public HookDirectFarCall
{
public:
	HookDirectFarCallPtr<TPtr>( const TPtr* pArg,
															AsmFarCall* pAsmCall,
															const PROC pfWrapperFunction,
															const FunctionInfo info,
															F_Callback pfPreHookCallback = NULL )
		: HookDirectFarCall( pAsmCall, pfWrapperFunction, info, const_cast<TPtr*>(pArg), pfPreHookCallback )
		{}
	HookDirectFarCallPtr<TPtr>( AsmFarCall* pAsmCall,
															const PROC pfWrapperFunction,
															const FunctionInfo info,
															F_Callback pfPreHookCallback = NULL )
		: HookDirectFarCall( pAsmCall, pfWrapperFunction, info, const_cast<HookDirectFarCallPtr<TPtr>*>(this), pfPreHookCallback )
		{}
	~HookDirectFarCallPtr<TPtr>() {}
};
typedef HookDirectFarCallPtr<> HookDirectFarCallP;


class DLLEXPORT HookWrapper : public CodeAllocator
{
public:
	class WrapperImp : public CodeAllocator
	{
		SafeCallEx m_CallOnEntry;
		AsmMovIXA m_SaveWrapperPtr;
		SafeCall m_CallHookedFunction;
//		AsmPushPtr m_PushResult; //already on stack
		AsmPushPtr m_PushThisPtr;
		AsmPushPtr m_PushArgSize;
		AsmPushPtr m_PushDestructor;
		AsmPushPtr m_PushRetAddr;
		AsmPtrJmp m_DeleteThis;
	public:
		void* m_pWrapper;
		RegisterStateStorage m_RegSave;
		PROC m_pfOnEntry;
		typedef DWORD_PTR (*_F_ONEXIT)( void*, DWORD_PTR );
		_F_ONEXIT m_pfOnExit;
	public:
#pragma warning(push)
#pragma warning(disable : 4355)
		WrapperImp( const PROC pfHookedFunction,
								const PROC pfOnEntry,
								const PROC pfOnExit,
								LPVOID pvReturnAddress )
			: m_CallOnEntry( pfOnEntry, m_RegSave )
			, m_SaveWrapperPtr( (DWORD_PTR)&m_pWrapper )
			, m_CallHookedFunction( pfHookedFunction )
			, m_PushThisPtr( this )
			, m_PushArgSize( sizeof(DWORD_PTR) * 2 )
			, m_PushDestructor( (LPVOID)Delete )
			, m_PushRetAddr( pvReturnAddress )
			, m_DeleteThis( (LPVOID)PreserveRegsStdCall )
			, m_pWrapper( NULL )
			, m_pfOnEntry( pfOnEntry )
			, m_pfOnExit( (_F_ONEXIT)pfOnExit )
			{}
#pragma warning(pop)
		static WrapperImp* __stdcall New( va_list args
																		/*const PROC pfHookedFunction,
																			const PROC pfOnEntry,
																			const PROC pfOnExit,
																			LPVOID pvReturnAddress*/ );
		static DWORD_PTR __stdcall Delete( va_list args
																		 /*WrapperImp* pThis,
																			 const DWORD_PTR dwResult*/ );
	};
#ifdef _WIN64
	typedef WrapperImp MemberWrapperImp;
#else
	class MemberWrapperImp : public CodeAllocator
	{
		AsmPushC m_SaveThis;
		AsmXchgCISPX m_StoreThis;
		AsmXchgCISP m_RestoreRetAddr;
		SafeCallEx m_CallOnEntry;
		AsmMovIXA m_SaveWrapperPtr;
		AsmPopA m_FixStackPtr;
		SafeCall m_CallHookedFunction;
//		AsmPushPtr m_PushResult; //already on stack
		AsmPushPtr m_PushThisPtr;
		AsmPushPtr m_PushArgSize;
		AsmPushPtr m_PushDestructor;
		AsmPushPtr m_PushRetAddr;
		AsmPtrJmp m_DeleteThis;
	public:
		void* m_pWrapper;
		RegisterStateStorage m_RegSave;
		PROC m_pfOnEntry;
		typedef DWORD_PTR (*_F_ONEXIT)( void*, DWORD_PTR );
		_F_ONEXIT m_pfOnExit;
	public:
#pragma warning(push)
#pragma warning(disable : 4355)
		MemberWrapperImp( const PROC pfHookedFunction,
											const PROC pfOnEntry,
											const PROC pfOnExit,
											LPVOID pvReturnAddress )
			: m_StoreThis( sizeof(DWORD_PTR) )
			, m_CallOnEntry( pfOnEntry, m_RegSave )
			, m_SaveWrapperPtr( (DWORD_PTR)&m_pWrapper )
			, m_CallHookedFunction( pfHookedFunction )
			, m_PushThisPtr( this )
			, m_PushArgSize( sizeof(DWORD_PTR) * 2 )
			, m_PushDestructor( (LPVOID)Delete )
			, m_PushRetAddr( pvReturnAddress )
			, m_DeleteThis( (LPVOID)PreserveRegsStdCall )
			, m_pWrapper( NULL )
			, m_pfOnEntry( pfOnEntry )
			, m_pfOnExit( (_F_ONEXIT)pfOnExit )
			{}
#pragma warning(pop)
		static MemberWrapperImp* __stdcall New( va_list args
																			/*const PROC pfHookedFunction,
																				const PROC pfOnEntry,
																				const PROC pfOnExit,
																				LPVOID pvReturnAddress*/ );
		static DWORD_PTR __stdcall Delete( va_list args
																			/*MemberWrapperImp* pThis,
																				const DWORD_PTR dwResult*/ );
	};
#endif
protected:
	AsmPushPtr m_PushOnExit;
	AsmPushPtr m_PushOnEntry;
	AsmPushPtr m_PushTargetFunction;
	AsmPushPtr m_ArgSize;
	AsmPushPtr m_ConstructorProc;
	AsmPtrCall m_MakeWrapperImp;
	AsmJmpA m_JumpToWrapper;
public:
	HookWrapper( const PROC pfTarget, const PROC pfOnEntry, const PROC pfOnExit, bool bMemberFunction = false )
	: m_PushOnExit( (DWORD_PTR)pfOnExit )
	, m_PushOnEntry( (DWORD_PTR)pfOnEntry )
	, m_PushTargetFunction( (DWORD_PTR)pfTarget )
	, m_ArgSize( sizeof(DWORD_PTR) * 3 )
	, m_ConstructorProc( bMemberFunction? (LPVOID)MemberWrapperImp::New : (LPVOID)WrapperImp::New )
	, m_MakeWrapperImp( PreserveRegsStdCallRet )
	{}
};


struct NullFunctionWrapper
{
	static bool IsMemberFunction() { return false; }
	static NullFunctionWrapper* OnEntry( ... ) { return NULL; }
	static DWORD_PTR OnExit( NullFunctionWrapper* pThis, const DWORD_PTR dwResult );
};


struct NullMemberFunctionWrapper
{
	static bool IsMemberFunction() { return true; }
	static NullFunctionWrapper* OnEntry( ... ) { return NULL; }
	static DWORD_PTR OnExit( NullFunctionWrapper* pThis, const DWORD_PTR dwResult );
};


class WrapCallBase : public HookCode
{
private:
	HookWrapper m_Wrapper;
	PROC m_pfMain;
public:
	WrapCallBase( LPVOID pAsmCall, const PROC pfOnEntry, const PROC pfOnExit );
	PROC GetOriginalDestination() const { return m_pfMain; }
};


template< typename _CWrapper = NullFunctionWrapper >
class DLLEXPORT WrapCall : public WrapCallBase
{
public:
	WrapCall( LPCVOID pAsmCall )
		:	WrapCallBase( pAsmCall,
										reinterpret_cast<PROC>(&_CWrapper::OnEntry),
										reinterpret_cast<PROC>(&_CWrapper::OnExit),
										_CWrapper::IsMemberFunction() )
		{}
};


class WrapCallTableEntryBase : public HookCode
{
private:
	HookWrapper m_Wrapper;
	PROC m_pfMain;
public:
	WrapCallTableEntryBase( const PROC* pEntry, const PROC pfOnEntry, const PROC pfOnExit, bool bMemberFunction = false );
	PROC GetOriginalDestination() const { return m_pfMain; }
};


template< typename _CWrapper = NullFunctionWrapper >
class DLLEXPORT WrapCallTableEntry : public WrapCallTableEntryBase
{
public:
	WrapCallTableEntry( LPCVOID pAsmCall )
		:	WrapCallTableEntryBase( pAsmCall,
															reinterpret_cast<PROC>(&_CWrapper::OnEntry),
															reinterpret_cast<PROC>(&_CWrapper::OnExit),
															_CWrapper::IsMemberFunction() )
		{}
};


class DLLEXPORT WrapFunctionEntryBase : public HookCode
{
private:
	HookWrapper m_Wrapper;
	StaticCodeProc m_OriginalFunction;
public:
	typedef bool (*F_Callback)( WrapFunctionEntryBase* pThis );

	WrapFunctionEntryBase( const PROC pfTargetFunction,
												 const ULONG cbOpCodes,
												 const PROC pfOnEntry,
												 const PROC pfOnExit,
												 F_Callback pfPreHookCallback = NULL,
												 bool bMemberFunction = false );
	FARPROC GetOriginalCode() const { return (FARPROC)m_OriginalFunction.GetOriginalCode(); }
};


template< typename _CWrapper = NullFunctionWrapper >
class DLLEXPORT WrapFunctionEntry : public WrapFunctionEntryBase
{
public:
	WrapFunctionEntry( const PROC pfTargetFunction, const ULONG cbOpCodes, F_Callback pfPreHookCallback = NULL )
		:	WrapFunctionEntryBase( pfTargetFunction,
														 cbOpCodes,
														 reinterpret_cast<PROC>(&_CWrapper::OnEntry),
														 reinterpret_cast<PROC>(&_CWrapper::OnExit),
														 pfPreHookCallback,
														 _CWrapper::IsMemberFunction() )
		{}
};


class DLLEXPORT HookCallTable : public HookCode
{
protected:
	PROC m_pfPre;
	PROC m_pfMain;
	PROC m_pfPost;
	CallRouter m_CallRouter;
public:
	HookCallTable( const PROC* pEntry,
								 const PROC pfPre = NULL,
								 const PROC pfPost = NULL );
	PROC GetOriginalDestination() const { return m_pfMain; }
};


class DLLEXPORT HookJumpTable : public HookCode
{
	AsmPtrCall m_HookExec;
	AsmPtrJmp m_OriginalCodeJmp;
public:
					HookJumpTable( PROC* pEntry,
												 const PROC pfHook = NULL );
};


class DLLEXPORT PtrTablePatch : public HookCode
{
	LPVOID const* m_rpvTable;
	ULONG m_ctEntries;
public:
	PtrTablePatch( LPVOID const* rpvTable, ULONG ctEntries );
	bool Restore( int idxTarget );
	bool RestoreAll();
	bool Set( int idxTarget, const void* pvNew );
	LPVOID const* GetOriginal() const { return (LPVOID const*)GetSavedCode(); }
	LPVOID const* GetCurrent() const { return m_rpvTable; }
	ULONG GetSize() const { return m_ctEntries; }
};


class DLLEXPORT PatchCode : public HookCode
{
	AsmPushPtr m_PushReturn;
	AsmPtrJmp m_PatchExec;
public:
					PatchCode( LPVOID pvTarget,
										 const ULONG cbOpCodes,
										 const PROC pfHook = NULL );
};


class DLLEXPORT PatchIndirectFarCall : public HookCode
{
public:
					PatchIndirectFarCall( LPVOID pvTarget,
																const PROC* ppfHook = NULL );
};


class DLLEXPORT NopPatch : public HookCode
{
public:
	NopPatch( LPVOID pvTarget, ULONG cbOpCodes );
};


#ifndef _WIN64
#define HOOKCODE_PROLOG\
		__asm push	ebp\
		__asm mov		ebp, esp\
		__asm sub		esp, __LOCAL_SIZE\
		__asm push  eax\
		__asm push  ebx\
		__asm push  ecx\
		__asm push  edx\
		__asm push  edi\
		__asm push  esi\
		__asm pushfd

#define HOOKCODE_EPILOG\
		__asm popfd\
		__asm pop   esi\
		__asm pop   edi\
		__asm pop   edx\
		__asm pop   ecx\
		__asm pop   ebx\
		__asm pop   eax\
		__asm mov		esp, ebp\
		__asm pop		ebp

#define HOOKCODE_RETURN\
		__asm ret

#define HOOKCODE_RETURN_N(n)\
		__asm ret		n

#define HOOKCODE_SETEAX(n)\
		__asm mov		eax, n\
		__asm mov		ebx, ebp\
		__asm sub		ebx, __LOCAL_SIZE\
		__asm mov		dword ptr [ebx - 4], eax

#define HOOKCODE_SETEBX(n)\
		__asm mov		eax, n\
		__asm mov		ebx, ebp\
		__asm sub		ebx, __LOCAL_SIZE\
		__asm mov		dword ptr [ebx - 8], eax

#define HOOKCODE_SETECX(n)\
		__asm mov		eax, n\
		__asm mov		ebx, ebp\
		__asm sub		ebx, __LOCAL_SIZE\
		__asm mov		dword ptr [ebx - 0xc], eax

#define HOOKCODE_SETEDX(n)\
		__asm mov		eax, n\
		__asm mov		ebx, ebp\
		__asm sub		ebx, __LOCAL_SIZE\
		__asm mov		dword ptr [ebx - 0x10], eax

#define HOOKCODE_SETEDI(n)\
		__asm mov		eax, n\
		__asm mov		ebx, ebp\
		__asm sub		ebx, __LOCAL_SIZE\
		__asm mov		dword ptr [ebx - 0x14], eax

#define HOOKCODE_SETESI(n)\
		__asm mov		eax, n\
		__asm mov		ebx, ebp\
		__asm sub		ebx, __LOCAL_SIZE\
		__asm mov		dword ptr [ebx - 0x18], eax

#define CALLHOOK_SKIPNEXTCALL\
		__asm add		dword ptr [ebp + 4], 14 /*sizeof(SafeCall)*/
#endif //_WIN64

#pragma pack(pop)

#endif //_ASMHOOK_H_
