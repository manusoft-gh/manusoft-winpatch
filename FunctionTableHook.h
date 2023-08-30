// Copyright 2010 ManuSoft
// https://www.manusoft.com
// All Rights Reserved
//
// Use of this software is governed by the terms of:
// GNU General Public License v2.0
// See LICENSE for details

#pragma once

#ifndef _FUNCTIONTABLEHOOK_H_
#define _FUNCTIONTABLEHOOK_H_

#include "Patch.h"
#include "AsmHook.h"


#ifndef DLLEXPORT
#define DLLEXPORT
#endif

#ifndef assert
#define assert(_Expression)     ((void)0)
#endif //asert


class DLLEXPORT FunctionTableHookBase
{
protected:
	FARPROC* mppfTarget;
	PatchT< FARPROC >* mpThisHook;
	FARPROC mpfChain;

public:
	FunctionTableHookBase( FARPROC* ppfTarget, FARPROC pfHook = NULL )
		: mppfTarget( ppfTarget )
		, mpThisHook( NULL )
		, mpfChain( ppfTarget? *ppfTarget : NULL )
		{
			if( ppfTarget && pfHook )
				hook( pfHook );
		}
	virtual ~FunctionTableHookBase()
		{
			unhook();
		}

	FARPROC* target() const { return mppfTarget; }
	FARPROC chain() const { return mpfChain; }
	bool isHooked() const { return (NULL != mpThisHook); }

protected:
	bool hook( FARPROC pfHook )
		{
			return ((mpThisHook = new PatchT< FARPROC >( mppfTarget, pfHook )) != NULL);
		}
	bool unhook()
		{
			bool bResult = (mpThisHook != NULL);
			if( bResult )
			{
				delete mpThisHook;
				mpThisHook = NULL;
			}
			return bResult;
		}
};


//Hook class that passes TPtr* to hook function
template< typename TPtr >
class FunctionTableHookPtr : public FunctionTableHookBase
{
private:
	HookStubInsertArgs< TPtr >* mpHookStub;

protected:
	FARPROC entrypoint() const { return *mpHookStub; }

public:
	FunctionTableHookPtr< TPtr >( const TPtr* pArg, FARPROC* ppfTarget, const FunctionInfo info, FARPROC pfHook = NULL )
		: FunctionTableHookBase( ppfTarget )
		, mpHookStub( NULL )
		{
			if( pfHook && ppfTarget )
			{
				mpHookStub = new HookStubInsertArgs< TPtr >( *ppfTarget, pfHook, info, pArg );
				assert( mpHookStub != NULL );
				if( mpHookStub )
					hook( *mpHookStub );
			}
		}
	~FunctionTableHookPtr< TPtr >()
		{
			delete mpHookStub;
		}
};

#endif //_FUNCTIONTABLEHOOK_H_
