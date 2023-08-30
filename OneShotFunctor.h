// Copyright 2010 ManuSoft
// https://www.manusoft.com
// All Rights Reserved
//
// Use of this software is governed by the terms of:
// GNU General Public License v2.0
// See LICENSE for details

#pragma once

#ifndef _ONESHOTFUNCTOR_H_
#define _ONESHOTFUNCTOR_H_


#include "AsmHook.h"

//#pragma comment(lib,"WinPatch.lib")


#ifndef DLLEXPORT
#define DLLEXPORT
#endif


#pragma pack(push)
#pragma pack(1)

template<class Functor>
class OneShotFunctor
{
	AsmFarCall m_PushIP;
	AsmPopA m_GetEip;
	AsmSubAX m_FixPtr;
	AsmPushA m_PushPtr;
	AsmPtrJmp m_CallExec;
	public:
	OneShotFunctor() : m_PushIP( (DWORD)0 ), m_FixPtr( sizeof(AsmFarCall) ), m_CallExec( (LPVOID)Exec ) {}
private:
	static void Exec(OneShotFunctor<Functor>* pThis) { Functor()(); delete pThis; }
};

#pragma pack(pop)

#endif //_ONESHOTFUNCTOR_H_
