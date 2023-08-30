// Copyright 2010 ManuSoft
// https://www.manusoft.com
// All Rights Reserved
//
// Use of this software is governed by the terms of:
// GNU General Public License v2.0
// See LICENSE for details

#pragma once

#ifndef _ASM_H_
#define _ASM_H_

#include <New.h>

#ifndef DLLEXPORT
	#define DLLEXPORT
#endif

#ifndef _WIN64
	#define DWORD_PTR DWORD
	#define UINT_PTR UINT
	#define __unaligned
#endif

#ifndef _PTRDIFF_T_DEFINED
#ifdef  _WIN64
typedef __int64        ptrdiff_t;
#else
typedef int            ptrdiff_t;
#endif
#define _PTRDIFF_T_DEFINED
#endif

typedef signed char SBYTE;


enum Asm{ eNoError, eMemAlloc, eMemAccess, eBadFormat };


#pragma pack(push)
#pragma pack(1)


template< typename I >
inline bool CompareOpCodeBytes( const BYTE* pTest )
	{ return *(I::_TInstruction::_TOpCodes*)pTest == I::_TInstruction::_OpCodes; }

//class AsmNop
//{
//	const BYTE m_Instruction;
//	typedef OpCodes<cbOpCodes> _TOpCodes;
//private:
//	AsmNop& operator=( const AsmNop& ); //not allowed
//public:
//	AsmNop() : m_Instruction( 0x90 ) {}
//	operator const BYTE() const { return 0x90; }
//	operator const WORD() const { return 0x9090; }
//	operator const DWORD() const { return 0x90909090; }
//};


template<const unsigned int cbOpCodes>
class OpCodes
{
	BYTE rbOpCodes[cbOpCodes];
public:
	OpCodes( const DWORD_PTR dw1 );
	bool operator==( const DWORD_PTR dw1 );
	operator const BYTE*() const { return rbOpCodes; }
};
#ifdef _WIN64
inline OpCodes<8>::OpCodes( const DWORD_PTR dw1 ) { *(DWORD_PTR*)rbOpCodes = dw1; }
inline OpCodes<7>::OpCodes( const DWORD_PTR dw1 ) { *(DWORD*)rbOpCodes = (DWORD)dw1; *(WORD*)(&rbOpCodes[4]) = *(WORD*)(((BYTE*)&dw1)[4]); rbOpCodes[6] = (((BYTE*)&dw1)[6]); }
inline OpCodes<6>::OpCodes( const DWORD_PTR dw1 ) { *(DWORD*)rbOpCodes = (DWORD)dw1; *(WORD*)(&rbOpCodes[4]) = *(WORD*)(((BYTE*)&dw1)[4]); }
inline OpCodes<5>::OpCodes( const DWORD_PTR dw1 ) { *(DWORD*)rbOpCodes = (DWORD)dw1; rbOpCodes[4] = ((BYTE*)&dw1)[4]; }
inline OpCodes<4>::OpCodes( const DWORD_PTR dw1 ) { *(DWORD*)rbOpCodes = (DWORD)dw1; }
inline OpCodes<3>::OpCodes( const DWORD_PTR dw1 ) { *(WORD*)rbOpCodes = (WORD)dw1; rbOpCodes[2] = ((BYTE*)&dw1)[2]; }
inline OpCodes<2>::OpCodes( const DWORD_PTR dw1 ) { *(WORD*)rbOpCodes = (WORD)dw1; }
inline OpCodes<1>::OpCodes( const DWORD_PTR dw1 ) { rbOpCodes[0] = (BYTE)dw1; }
inline bool OpCodes<8>::operator==( const DWORD_PTR dw1 )
	{ return (*(DWORD_PTR*)rbOpCodes == dw1); }
inline bool OpCodes<7>::operator==( const DWORD_PTR dw1 )
	{ return (*(DWORD*)rbOpCodes == (DWORD)dw1); *(WORD*)(&rbOpCodes[4]) = *(WORD*)(((BYTE*)&dw1)[4]); rbOpCodes[6] = (((BYTE*)&dw1)[6]); }
inline bool OpCodes<6>::operator==( const DWORD_PTR dw1 )
	{ return (*(DWORD*)rbOpCodes == (DWORD)dw1); *(WORD*)(&rbOpCodes[4]) = *(WORD*)(((BYTE*)&dw1)[4]); }
inline bool OpCodes<5>::operator==( const DWORD_PTR dw1 )
	{ return (*(DWORD*)rbOpCodes == (DWORD)dw1); rbOpCodes[4] = ((BYTE*)&dw1)[4]; }
inline bool OpCodes<4>::operator==( const DWORD_PTR dw1 )
	{ return (*(DWORD*)rbOpCodes == (DWORD)dw1); }
inline bool OpCodes<3>::operator==( const DWORD_PTR dw1 )
	{ return (*(WORD*)rbOpCodes == (WORD)dw1 && rbOpCodes[2] == ((BYTE*)&dw1)[2]); }
inline bool OpCodes<2>::operator==( const DWORD_PTR dw1 )
	{ return (*(WORD*)rbOpCodes == (WORD)dw1); }
inline bool OpCodes<1>::operator==( const DWORD_PTR dw1 )
	{ return (rbOpCodes[0] == (BYTE)dw1); }
#else //_WIN64
//inline OpCodes<8>::OpCodes( const DWORD_PTR dw1 ) { *(DWORD_PTR*)rbOpCodes = dw1; }
//inline OpCodes<7>::OpCodes( const DWORD_PTR dw1 ) { *(DWORD*)rbOpCodes = dw1; *(WORD*)(&rbOpCodes[4]) = *(WORD*)(((BYTE*)&dw1)[4]); rbOpCodes[6] = (((BYTE*)&dw1)[6]); }
//inline OpCodes<6>::OpCodes( const DWORD_PTR dw1 ) { *(DWORD*)rbOpCodes = dw1; *(WORD*)(&rbOpCodes[4]) = *(WORD*)(((BYTE*)&dw1)[4]); }
//inline OpCodes<5>::OpCodes( const DWORD_PTR dw1 ) { *(DWORD*)rbOpCodes = dw1; rbOpCodes[4] = ((BYTE*)&dw1)[4]; }
inline OpCodes<4>::OpCodes( const DWORD_PTR dw1 ) { *(DWORD*)rbOpCodes = dw1; }
inline OpCodes<3>::OpCodes( const DWORD_PTR dw1 ) { *(WORD*)rbOpCodes = (WORD)dw1; rbOpCodes[2] = ((BYTE*)&dw1)[2]; }
inline OpCodes<2>::OpCodes( const DWORD_PTR dw1 ) { *(WORD*)rbOpCodes = (WORD)dw1; }
inline OpCodes<1>::OpCodes( const DWORD_PTR dw1 ) { rbOpCodes[0] = (BYTE)dw1; }
//inline bool OpCodes<8>::operator==( const DWORD_PTR dw1 )
//	{ return (*(DWORD_PTR*)rbOpCodes == dw1); }
//inline bool OpCodes<7>::operator==( const DWORD_PTR dw1 )
//	{ return (*(DWORD*)rbOpCodes == dw1); *(WORD*)(&rbOpCodes[4]) = *(WORD*)(((BYTE*)&dw1)[4]); rbOpCodes[6] = (((BYTE*)&dw1)[6]); }
//inline bool OpCodes<6>::operator==( const DWORD_PTR dw1 )
//	{ return (*(DWORD*)rbOpCodes == dw1); *(WORD*)(&rbOpCodes[4]) = *(WORD*)(((BYTE*)&dw1)[4]); }
//inline bool OpCodes<5>::operator==( const DWORD_PTR dw1 )
//	{ return (*(DWORD*)rbOpCodes == dw1); rbOpCodes[4] = ((BYTE*)&dw1)[4]; }
inline bool OpCodes<4>::operator==( const DWORD_PTR dw1 )
	{ return (*(DWORD*)rbOpCodes == dw1); }
inline bool OpCodes<3>::operator==( const DWORD_PTR dw1 )
	{ return (*(WORD*)rbOpCodes == (WORD)dw1 && rbOpCodes[2] == ((BYTE*)&dw1)[2]); }
inline bool OpCodes<2>::operator==( const DWORD_PTR dw1 )
	{ return (*(WORD*)rbOpCodes == (WORD)dw1); }
inline bool OpCodes<1>::operator==( const DWORD_PTR dw1 )
	{ return (rbOpCodes[0] == (BYTE)dw1); }
#endif //_WIN64

template<const unsigned int cbOpCodes, const DWORD_PTR Op>
class AsmInstruction
{
	OpCodes<cbOpCodes> m_Instruction;
public:
	AsmInstruction() : m_Instruction( Op ) {}
	AsmInstruction( const class AsmNop& Nop ) : m_Instruction( Nop ) {}
	enum{ _OpCodes = Op };
	typedef OpCodes<cbOpCodes> _TOpCodes;
	typedef AsmInstruction _TInstruction;
};


template<const unsigned int cbOpCodes, typename O, const DWORD_PTR Op, const O DefOp = (O)0>
class AsmOperandInstruction
{
	AsmInstruction<cbOpCodes,Op> m_Instruction;
	O m_Operand;
public:
					AsmOperandInstruction( const O opInit = DefOp ) : m_Operand( opInit ) {}
					AsmOperandInstruction( const AsmNop& Nop ) : m_Instruction( Nop ), m_Operand( Nop ) {}
	void		Set( const O opNew ) { m_Operand = opNew; }
	O				Get() const { return m_Operand; }
	enum{ _OpCodes = Op };
	typedef AsmInstruction<cbOpCodes,Op> _TInstruction;
	typedef struct{ O Op1; } _TOperands;
};


template<const unsigned int cbOpCodes, typename O1, typename O2, const DWORD_PTR Op,
				 const O1 DefOp1 = (O1)0, const O2 DefOp2 = (O2)0>
class Asm2OperandInstruction
{
	AsmInstruction<cbOpCodes,Op> m_Instruction;
	O1 m_Operand1;
	O2 m_Operand2;
public:
					Asm2OperandInstruction( const O1 opInit1 = DefOp1,  const O2 opInit2 = DefOp2 )
						: m_Operand1( opInit1 ), m_Operand2( opInit2 ) {}
					Asm2OperandInstruction( const AsmNop& Nop )
						: m_Instruction( Nop ), m_Operand1( Nop ), m_Operand2( Nop ) {}
	void		Set( const O1 opNew1, const O2 opNew2 )
						{ m_Operand1 = opNew1; m_Operand2 = opNew2; }
	O1			Get1() const { return m_Operand1; }
	O2			Get2() const { return m_Operand2; }
	enum{ _OpCodes = Op };
	typedef AsmInstruction<cbOpCodes,Op> _TInstruction;
	typedef struct{ O1 Op1; O2 Op2; } _TOperands;
};


template<const unsigned int cbOpCodes, const DWORD_PTR Op, typename Ofs = LONG>
class AsmRelativeOperandInstruction : public AsmOperandInstruction<cbOpCodes,Ofs,Op>
{
public:
					AsmRelativeOperandInstruction( __unaligned LPCVOID pvDestination );
					AsmRelativeOperandInstruction( const Ofs dwOffset );
					AsmRelativeOperandInstruction( const AsmNop& Nop );
	void		Release();
	void		SetDestination( __unaligned LPCVOID pvDestination );
	__unaligned LPCVOID	GetDestination() const;
	typedef AsmOperandInstruction<cbOpCodes,Ofs,Op>::_TInstruction _TInstruction;
	typedef AsmOperandInstruction<cbOpCodes,Ofs,Op>::_TOperands _TOperands;
};


class AsmNop : public AsmInstruction<1,0x90>
{
private:
	AsmNop& operator=( const AsmNop& ); //not allowed
public:
	AsmNop() {}
	operator const BYTE() const { return 0x90; }
	operator const WORD() const { return 0x9090; }
	operator const DWORD() const { return 0x90909090; }
};



#ifdef _WIN64
typedef AsmRelativeOperandInstruction<2,0x25ff,LONG> AsmFarJmpIndirect;
typedef AsmRelativeOperandInstruction<2,0x15ff,LONG> AsmFarCallIndirect;

typedef AsmInstruction<1,0x50> AsmPushRax;
typedef AsmInstruction<1,0x51> AsmPushRcx;
typedef AsmInstruction<1,0x52> AsmPushRdx;
typedef AsmInstruction<1,0x53> AsmPushRbx;
typedef AsmInstruction<1,0x54> AsmPushRsp;
typedef AsmInstruction<1,0x55> AsmPushRbp;
typedef AsmInstruction<1,0x56> AsmPushRsi;
typedef AsmInstruction<1,0x57> AsmPushRdi;
typedef AsmInstruction<2,0x5041> AsmPushR8;
typedef AsmInstruction<2,0x5141> AsmPushR9;
typedef AsmInstruction<2,0x5241> AsmPushR10;
typedef AsmInstruction<2,0x5341> AsmPushR11;
typedef AsmInstruction<1,0x58> AsmPopRax;
typedef AsmInstruction<1,0x59> AsmPopRcx;
typedef AsmInstruction<1,0x5a> AsmPopRdx;
typedef AsmInstruction<1,0x5b> AsmPopRbx;
typedef AsmInstruction<1,0x5c> AsmPopRsp;
typedef AsmInstruction<1,0x5d> AsmPopRbp;
typedef AsmInstruction<1,0x5e> AsmPopRsi;
typedef AsmInstruction<1,0x5f> AsmPopRdi;
typedef AsmInstruction<2,0x5841> AsmPopR8;
typedef AsmInstruction<2,0x5941> AsmPopR9;
typedef AsmInstruction<2,0x5a41> AsmPopR10;
typedef AsmInstruction<2,0x5b41> AsmPopR11;

typedef struct
{
	AsmPushRax A;
	AsmPushRcx C;
	AsmPushRdx D;
	AsmPushRbx B;
	AsmPushRbp BP;
	AsmPushRsi SI;
	AsmPushRdi DI;
	AsmPushR8 R8;
	AsmPushR9 R9;
	AsmPushR10 R10;
	AsmPushR11 R11;
} AsmPushAQ;
typedef struct
{
	AsmPopR11 R11;
	AsmPopR10 R10;
	AsmPopR9 R9;
	AsmPopR8 R8;
	AsmPopRdi DI;
	AsmPopRsi SI;
	AsmPopRbp BP;
	AsmPopRbx B;
	AsmPopRdx D;
	AsmPopRcx C;
	AsmPopRax A;
} AsmPopAQ;
typedef AsmInstruction<1,0x9c> AsmPushFQ;
typedef AsmInstruction<1,0x9d> AsmPopFQ;

typedef AsmInstruction<3,0xc03348> AsmXorRaxRax;
typedef AsmInstruction<3,0xdb3348> AsmXorRbxRbx;
typedef AsmInstruction<3,0xc93348> AsmXorRcxRcx;

typedef AsmInstruction<3,0xc08548> AsmTestRaxRax;
typedef AsmInstruction<3,0xdb8548> AsmTestRbxRbx;
typedef AsmInstruction<3,0xc98548> AsmTestRcxRcx;
typedef AsmInstruction<3,0xd28548> AsmTestRdxRdx;
typedef AsmInstruction<3,0xf68548> AsmTestRsiRsi;
typedef AsmInstruction<3,0xff8548> AsmTestRdiRdi;
typedef AsmInstruction<3,0xc0854d> AsmTestR8R8;

typedef AsmInstruction<3,0xc0ff48> AsmIncRax;
typedef AsmInstruction<3,0xc1ff48> AsmIncRcx;
typedef AsmInstruction<3,0xc2ff48> AsmIncRdx;
typedef AsmInstruction<3,0xc3ff48> AsmIncRbx;
typedef AsmInstruction<3,0xc4ff48> AsmIncRsp;
typedef AsmInstruction<3,0xc5ff48> AsmIncRbp;
typedef AsmInstruction<3,0xc6ff48> AsmIncRsi;
typedef AsmInstruction<3,0xc7ff48> AsmIncRdi;
typedef AsmInstruction<3,0xc8ff48> AsmDecRax;
typedef AsmInstruction<3,0xc9ff48> AsmDecRcx;
typedef AsmInstruction<3,0xcaff48> AsmDecRdx;
typedef AsmInstruction<3,0xcbff48> AsmDecRbx;
typedef AsmInstruction<3,0xccff48> AsmDecRsp;
typedef AsmInstruction<3,0xcdff48> AsmDecRbp;
typedef AsmInstruction<3,0xceff48> AsmDecRsi;
typedef AsmInstruction<3,0xcfff48> AsmDecRdi;

typedef AsmOperandInstruction<2,DWORD_PTR,0xb848> AsmMovRaxX;
typedef AsmOperandInstruction<2,DWORD_PTR,0xb948> AsmMovRcxX;
typedef AsmOperandInstruction<2,DWORD_PTR,0xba48> AsmMovRdxX;
typedef AsmOperandInstruction<2,DWORD_PTR,0xbb48> AsmMovRbxX;
typedef AsmOperandInstruction<2,DWORD_PTR,0xbc48> AsmMovRspX;
typedef AsmOperandInstruction<2,DWORD_PTR,0xbd48> AsmMovRbpX;
typedef AsmOperandInstruction<2,DWORD_PTR,0xbe48> AsmMovRsiX;
typedef AsmOperandInstruction<2,DWORD_PTR,0xbf48> AsmMovRdiX;

typedef AsmInstruction<3,0xec8b48> AsmMovRbpRsp;
typedef AsmInstruction<3,0xe58b48> AsmMovRspRbp;
typedef AsmInstruction<3,0xc18948> AsmMovRcxRax;
typedef AsmInstruction<3,0xc88948> AsmMovRaxRcx;
typedef AsmInstruction<3,0xe08948> AsmMovRaxRsp;
typedef AsmInstruction<3,0xc48948> AsmMovRspRax;
typedef AsmInstruction<3,0xca8948> AsmMovRdxRcx;
typedef AsmInstruction<3,0xd08949> AsmMovR8Rdx;
typedef AsmInstruction<3,0xc08949> AsmMovR8Rcx;
typedef AsmInstruction<3,0xd18949> AsmMovR9Rdx;
typedef AsmInstruction<3,0xc18949> AsmMovR9Rcx;
typedef AsmInstruction<3,0xc1894d> AsmMovR9R8;
typedef AsmInstruction<3,0xca894d> AsmMovR10R9;

typedef AsmInstruction<3,0x008b48> AsmMovRaxIRax;
typedef AsmInstruction<3,0x038b48> AsmMovRaxIRbx;
typedef AsmOperandInstruction<3,BYTE,0x5b8b48> AsmMovRbxIRbxX;
typedef AsmOperandInstruction<3,BYTE,0x408b48> AsmMovRaxIRaxX;
typedef AsmOperandInstruction<4,BYTE,0x24448b48> AsmMovRaxIRspX;
typedef AsmOperandInstruction<4,BYTE,0x244c8b48> AsmMovRcxIRspX;
typedef AsmOperandInstruction<4,BYTE,0x24548b48> AsmMovRdxIRspX;
typedef AsmOperandInstruction<4,BYTE,0x245c8b48> AsmMovRbxIRspX;
typedef AsmOperandInstruction<4,BYTE,0x24448b4c> AsmMovR8IRspX;
typedef AsmOperandInstruction<4,BYTE,0x244c8b4c> AsmMovR9IRspX;
typedef AsmOperandInstruction<4,BYTE,0x24548b4c> AsmMovR10IRspX;
typedef AsmOperandInstruction<4,BYTE,0x245c8b4c> AsmMovR11IRspX;
typedef AsmOperandInstruction<4,BYTE,0x24448d48> AsmLeaRaxIRspX;
typedef AsmOperandInstruction<4,BYTE,0x244c8d48> AsmLeaRcxIRspX;
typedef AsmOperandInstruction<4,BYTE,0x24548d48> AsmLeaRdxIRspX;
typedef AsmOperandInstruction<4,BYTE,0x245c8d48> AsmLeaRbxIRspX;

typedef AsmOperandInstruction<4,DWORD,0x2404c748> AsmMovIRsp0X;

typedef AsmOperandInstruction<4,BYTE,0x24448948> AsmMovIRspXRax;
typedef AsmOperandInstruction<4,BYTE,0x24448948,0> AsmMovIRspRax;
typedef AsmOperandInstruction<4,BYTE,0x244c8948> AsmMovIRspXRcx;
typedef AsmOperandInstruction<4,BYTE,0x24548948> AsmMovIRspXRdx;
typedef AsmOperandInstruction<4,BYTE,0x245c8948> AsmMovIRspXRbx;
typedef AsmOperandInstruction<4,BYTE,0x2444894c> AsmMovIRspXR8;
typedef AsmOperandInstruction<4,BYTE,0x244c894c> AsmMovIRspXR9;
typedef AsmOperandInstruction<4,BYTE,0x2454894c> AsmMovIRspXR10;
typedef AsmOperandInstruction<4,BYTE,0x245c894c> AsmMovIRspXR11;

typedef AsmOperandInstruction<3,BYTE,0x458948> AsmMovIRbpXRax;
typedef AsmOperandInstruction<3,BYTE,0x408948> AsmMovIRaxXRax;

typedef AsmOperandInstruction<2,DWORD_PTR,0xa348> AsmMovIXRax;
typedef AsmInstruction<4,0x24048748> AsmXchgRaxIRsp;
typedef AsmInstruction<4,0x240c8748> AsmXchgRcxIRsp;
typedef AsmOperandInstruction<4,BYTE,0x24448748> AsmXchgRaxIRspX;
typedef AsmOperandInstruction<4,BYTE,0x244c8748> AsmXchgRcxIRspX;
typedef AsmOperandInstruction<3,BYTE,0xe88348> AsmSubRaxX;
typedef AsmOperandInstruction<3,BYTE,0xec8348> AsmSubRspX;
typedef AsmInstruction<4,0x08ec8348> AsmSubRsp8;
typedef AsmInstruction<4,0x10ec8348> AsmSubRsp10;
typedef AsmInstruction<4,0x18ec8348> AsmSubRsp18;
typedef AsmInstruction<4,0x20ec8348> AsmSubRsp20;
typedef AsmOperandInstruction<3,BYTE,0xc48348> AsmAddRspX;
typedef AsmInstruction<4,0x08c48348> AsmAddRsp8;
typedef AsmInstruction<4,0x10c48348> AsmAddRsp10;
typedef AsmInstruction<4,0x18c48348> AsmAddRsp18;
typedef AsmInstruction<4,0x20c48348> AsmAddRsp20;

typedef AsmInstruction<3,0xe0ff48> AsmJmpRax;
typedef AsmInstruction<5,0x002464ff48> AsmJmpIRsp;
typedef AsmOperandInstruction<4,BYTE,0x2464ff48> AsmJmpIRspX;


//architecture-independent typedefs
typedef AsmPushRax AsmPushA;
typedef AsmPushRcx AsmPushC;
typedef AsmPushRdx AsmPushD;
typedef AsmPushRbx AsmPushB;
typedef AsmPushRsp AsmPushSP;
typedef AsmPushRbp AsmPushBP;
typedef AsmPushRsi AsmPushSI;
typedef AsmPushRdi AsmPushDI;
typedef AsmPopRax AsmPopA;
typedef AsmPopRcx AsmPopC;
typedef AsmPopRdx AsmPopD;
typedef AsmPopRbx AsmPopB;
typedef AsmPopRsp AsmPopSP;
typedef AsmPopRbp AsmPopBP;
typedef AsmPopRsi AsmPopSI;
typedef AsmPopRdi AsmPopDI;

typedef AsmPushAQ AsmPushAll;
typedef AsmPopAQ AsmPopAll;
typedef AsmPushFQ AsmPushF;
typedef AsmPopFQ AsmPopF;

typedef AsmXorRaxRax AsmXorAA;
typedef AsmXorRbxRbx AsmXorBB;
typedef AsmXorRcxRcx AsmXorCC;

typedef AsmTestRaxRax AsmTestAA;
typedef AsmTestRbxRbx AsmTestBB;
typedef AsmTestRcxRcx AsmTestCC;
typedef AsmTestRdxRdx AsmTestDD;
typedef AsmTestRsiRsi AsmTestSISI;
typedef AsmTestRdiRdi AsmTestDIDI;

typedef AsmIncRax AsmIncA;
typedef AsmIncRcx AsmIncC;
typedef AsmIncRdx AsmIncD;
typedef AsmIncRbx AsmIncB;
typedef AsmIncRsp AsmIncSP;
typedef AsmIncRbp AsmIncBP;
typedef AsmIncRsi AsmIncSI;
typedef AsmIncRdi AsmIncDI;
typedef AsmDecRax AsmDecA;
typedef AsmDecRcx AsmDecC;
typedef AsmDecRdx AsmDecD;
typedef AsmDecRbx AsmDecB;
typedef AsmDecRsp AsmDecSP;
typedef AsmDecRbp AsmDecBP;
typedef AsmDecRsi AsmDecSI;
typedef AsmDecRdi AsmDecDI;

typedef AsmMovRaxX AsmMovAX;
typedef AsmMovRcxX AsmMovCX;
typedef AsmMovRdxX AsmMovDX;
typedef AsmMovRbxX AsmMovBX;
typedef AsmMovRspX AsmMovSPX;
typedef AsmMovRbpX AsmMovBPX;
typedef AsmMovRsiX AsmMovSIX;
typedef AsmMovRdiX AsmMovDIX;

typedef AsmMovRbpRsp AsmMovBPSP;
typedef AsmMovRspRbp AsmMovSPBP;
typedef AsmMovRcxRax AsmMovCA;
typedef AsmMovRaxRcx AsmMovAC;
typedef AsmMovRaxRsp AsmMovASP;
typedef AsmMovRspRax AsmMovSPA;

typedef AsmMovRaxIRax AsmMovAIA;
typedef AsmMovRaxIRbx AsmMovAIB;
typedef AsmMovRbxIRbxX AsmMovBIBX;
typedef AsmMovRaxIRaxX AsmMovAIAX;
typedef AsmMovRaxIRspX AsmMovAISPX;
typedef AsmMovRcxIRspX AsmMovCISPX;
typedef AsmMovRdxIRspX AsmMovDISPX;
typedef AsmMovRbxIRspX AsmMovBISPX;
typedef AsmMovR8IRspX AsmMovR8ISPX;
typedef AsmMovR9IRspX AsmMovR9ISPX;
typedef AsmMovR10IRspX AsmMovR10ISPX;
typedef AsmMovR11IRspX AsmMovR11ISPX;
typedef AsmLeaRaxIRspX AsmLeaAISPX;
typedef AsmLeaRcxIRspX AsmLeaCISPX;
typedef AsmLeaRdxIRspX AsmLeaDISPX;
typedef AsmLeaRbxIRspX AsmLeaBISPX;

typedef AsmMovIRsp0X AsmMovISP0X;

typedef AsmMovIRspXRax AsmMovISPXA;
typedef AsmMovIRspRax AsmMovISPA;
typedef AsmMovIRspXRcx AsmMovISPXC;
typedef AsmMovIRspXRdx AsmMovISPXD;
typedef AsmMovIRspXRbx AsmMovISPXB;
typedef AsmMovIRspXR8 AsmMovISPXR8;
typedef AsmMovIRspXR9 AsmMovISPXR9;
typedef AsmMovIRspXR10 AsmMovISPXR10;
typedef AsmMovIRspXR10 AsmMovISPXR10;

typedef AsmMovIRbpXRax AsmMovIBPXA;
typedef AsmMovIRaxXRax AsmMovIAXA;

typedef AsmMovIXRax AsmMovIXA;
typedef AsmXchgRaxIRsp AsmXchgAISP;
typedef AsmXchgRcxIRsp AsmXchgCISP;
typedef AsmXchgRaxIRspX AsmXchgAISPX;
typedef AsmXchgRcxIRspX AsmXchgCISPX;
typedef AsmSubRaxX AsmSubAX;
typedef AsmSubRspX AsmSubSPX;
typedef AsmAddRspX AsmAddSPX;

typedef AsmJmpRax AsmJmpA;
typedef AsmJmpIRsp AsmJmpISP;
typedef AsmJmpIRspX AsmJmpISPX;
#else
#pragma warning(push)
#pragma warning(disable:4097)
struct AsmFarJmpIndirect : public AsmOperandInstruction<2,DWORD,0x25ff>
{
	typedef AsmOperandInstruction<2,DWORD,0x25ff> _Base;
	typedef _Base::_TInstruction _TInstruction;
	AsmFarJmpIndirect( LPCVOID pvDest ) : _Base( (DWORD_PTR)pvDest ) {}
	AsmFarJmpIndirect( const DWORD opInit = 0 ) : _Base( opInit ) {}
	AsmFarJmpIndirect( const AsmNop& Nop ) : _Base( Nop ) {}
	void SetDestination( __unaligned LPCVOID pvDestination ) { Set( (DWORD)pvDestination ); }
	__unaligned LPCVOID	GetDestination() const { return (LPCVOID)Get(); }
};
struct AsmFarCallIndirect : public AsmOperandInstruction<2,DWORD,0x15ff>
{
	typedef AsmOperandInstruction<2,DWORD,0x15ff> _Base;
	typedef _Base::_TInstruction _TInstruction;
	AsmFarCallIndirect( LPCVOID pvDest ) : _Base( (DWORD)pvDest ) {}
	AsmFarCallIndirect( const DWORD opInit = 0 ) : _Base( opInit ) {}
	AsmFarCallIndirect( const AsmNop& Nop ) : _Base( Nop ) {}
	void SetDestination( __unaligned LPCVOID pvDestination ) { Set( (DWORD)pvDestination ); }
	__unaligned LPCVOID	GetDestination() const { return (LPCVOID)Get(); }
};
#pragma warning(pop)

typedef AsmInstruction<1,0x50> AsmPushEax;
typedef AsmInstruction<1,0x51> AsmPushEcx;
typedef AsmInstruction<1,0x52> AsmPushEdx;
typedef AsmInstruction<1,0x53> AsmPushEbx;
typedef AsmInstruction<1,0x54> AsmPushEsp;
typedef AsmInstruction<1,0x55> AsmPushEbp;
typedef AsmInstruction<1,0x56> AsmPushEsi;
typedef AsmInstruction<1,0x57> AsmPushEdi;
typedef AsmInstruction<1,0x58> AsmPopEax;
typedef AsmInstruction<1,0x59> AsmPopEcx;
typedef AsmInstruction<1,0x5a> AsmPopEdx;
typedef AsmInstruction<1,0x5b> AsmPopEbx;
typedef AsmInstruction<1,0x5c> AsmPopEsp;
typedef AsmInstruction<1,0x5d> AsmPopEbp;
typedef AsmInstruction<1,0x5e> AsmPopEsi;
typedef AsmInstruction<1,0x5f> AsmPopEdi;

typedef AsmInstruction<1,0x60> AsmPushAD;
typedef AsmInstruction<1,0x61> AsmPopAD;
typedef AsmInstruction<1,0x9c> AsmPushFD;
typedef AsmInstruction<1,0x9d> AsmPopFD;

typedef AsmInstruction<2,0xc033> AsmXorEaxEax;
typedef AsmInstruction<2,0xdb33> AsmXorEbxEbx;
typedef AsmInstruction<2,0xc933> AsmXorEcxEcx;

typedef AsmInstruction<2,0xc085> AsmTestEaxEax;
typedef AsmInstruction<2,0xdb85> AsmTestEbxEbx;
typedef AsmInstruction<2,0xc985> AsmTestEcxEcx;
typedef AsmInstruction<2,0xd285> AsmTestEdxEdx;
typedef AsmInstruction<2,0xf685> AsmTestEsiEsi;
typedef AsmInstruction<2,0xff85> AsmTestEdiEdi;

typedef AsmInstruction<1,0x40> AsmIncEax;
typedef AsmInstruction<1,0x41> AsmIncEcx;
typedef AsmInstruction<1,0x42> AsmIncEdx;
typedef AsmInstruction<1,0x43> AsmIncEbx;
typedef AsmInstruction<1,0x44> AsmIncEsp;
typedef AsmInstruction<1,0x45> AsmIncEbp;
typedef AsmInstruction<1,0x46> AsmIncEsi;
typedef AsmInstruction<1,0x47> AsmIncEdi;
typedef AsmInstruction<1,0x48> AsmDecEax;
typedef AsmInstruction<1,0x49> AsmDecEcx;
typedef AsmInstruction<1,0x4a> AsmDecEdx;
typedef AsmInstruction<1,0x4b> AsmDecEbx;
typedef AsmInstruction<1,0x4c> AsmDecEsp;
typedef AsmInstruction<1,0x4d> AsmDecEbp;
typedef AsmInstruction<1,0x4e> AsmDecEsi;
typedef AsmInstruction<1,0x4f> AsmDecEdi;

typedef AsmOperandInstruction<1,DWORD_PTR,0xb8> AsmMovEaxX;
typedef AsmOperandInstruction<1,DWORD_PTR,0xb9> AsmMovEcxX;
typedef AsmOperandInstruction<1,DWORD_PTR,0xba> AsmMovEdxX;
typedef AsmOperandInstruction<1,DWORD_PTR,0xbb> AsmMovEbxX;
typedef AsmOperandInstruction<1,DWORD_PTR,0xbc> AsmMovEspX;
typedef AsmOperandInstruction<1,DWORD_PTR,0xbd> AsmMovEbpX;
typedef AsmOperandInstruction<1,DWORD_PTR,0xbe> AsmMovEsiX;
typedef AsmOperandInstruction<1,DWORD_PTR,0xbf> AsmMovEdiX;

typedef AsmInstruction<2,0xec8b> AsmMovEbpEsp;
typedef AsmInstruction<2,0xe58b> AsmMovEspEbp;
typedef AsmInstruction<2,0xc189> AsmMovEcxEax;
typedef AsmInstruction<2,0xc889> AsmMovEaxEcx;
typedef AsmInstruction<2,0xe089> AsmMovEaxEsp;
typedef AsmInstruction<2,0xc489> AsmMovEspEax;

typedef AsmInstruction<3,0x00408b> AsmMovEaxIEax;
typedef AsmInstruction<2,0x038b> AsmMovEaxIEbx;
typedef AsmOperandInstruction<2,BYTE,0x5b8b> AsmMovEbxIEbxX;
typedef AsmOperandInstruction<2,BYTE,0x408b> AsmMovEaxIEaxX;
typedef AsmOperandInstruction<3,BYTE,0x24448b> AsmMovEaxIEspX;
typedef AsmOperandInstruction<3,BYTE,0x244c8b> AsmMovEcxIEspX;
typedef AsmOperandInstruction<3,BYTE,0x24548b> AsmMovEdxIEspX;
typedef AsmOperandInstruction<3,BYTE,0x245c8b> AsmMovEbxIEspX;
typedef AsmOperandInstruction<3,BYTE,0x24448d> AsmLeaEaxIEspX;
typedef AsmOperandInstruction<3,BYTE,0x244c8d> AsmLeaEcxIEspX;
typedef AsmOperandInstruction<3,BYTE,0x24548d> AsmLeaEdxIEspX;
typedef AsmOperandInstruction<3,BYTE,0x245c8d> AsmLeaEbxIEspX;

typedef AsmOperandInstruction<3,DWORD,0x2404c7> AsmMovIEsp0X;

typedef AsmOperandInstruction<3,BYTE,0x244489> AsmMovIEspXEax;
typedef AsmOperandInstruction<3,BYTE,0x244489,0> AsmMovIEspEax;
typedef AsmOperandInstruction<3,BYTE,0x244c89> AsmMovIEspXEcx;
typedef AsmOperandInstruction<3,BYTE,0x245489> AsmMovIEspXEdx;
typedef AsmOperandInstruction<3,BYTE,0x245c89> AsmMovIEspXEbx;

typedef AsmOperandInstruction<2,BYTE,0x4589> AsmMovIEbpXEax;
typedef AsmOperandInstruction<2,BYTE,0x4089> AsmMovIEaxXEax;

typedef AsmOperandInstruction<1,DWORD,0xa3> AsmMovIXEax;
typedef AsmInstruction<3,0x240487> AsmXchgEaxIEsp;
typedef AsmInstruction<3,0x240c87> AsmXchgEcxIEsp;
typedef AsmOperandInstruction<3,BYTE,0x244487> AsmXchgEaxIEspX;
typedef AsmOperandInstruction<3,BYTE,0x244c87> AsmXchgEcxIEspX;
typedef AsmOperandInstruction<2,BYTE,0xe883> AsmSubEaxX;
typedef AsmOperandInstruction<2,BYTE,0xec83> AsmSubEspX;
typedef AsmOperandInstruction<2,BYTE,0xc483> AsmAddEspX;

typedef AsmInstruction<2,0xe0ff> AsmJmpEax;
typedef AsmInstruction<4,0x002464ff> AsmJmpIEsp;
typedef AsmOperandInstruction<3,BYTE,0x2464ff> AsmJmpIEspX;


//architecture-independent typedefs
typedef AsmPushEax AsmPushA;
typedef AsmPushEcx AsmPushC;
typedef AsmPushEdx AsmPushD;
typedef AsmPushEbx AsmPushB;
typedef AsmPushEsp AsmPushSP;
typedef AsmPushEbp AsmPushBP;
typedef AsmPushEsi AsmPushSI;
typedef AsmPushEdi AsmPushDI;
typedef AsmPopEax AsmPopA;
typedef AsmPopEcx AsmPopC;
typedef AsmPopEdx AsmPopD;
typedef AsmPopEbx AsmPopB;
typedef AsmPopEsp AsmPopSP;
typedef AsmPopEbp AsmPopBP;
typedef AsmPopEsi AsmPopSI;
typedef AsmPopEdi AsmPopDI;

typedef AsmPushAD AsmPushAll;
typedef AsmPopAD AsmPopAll;
typedef AsmPushFD AsmPushF;
typedef AsmPopFD AsmPopF;

typedef AsmXorEaxEax AsmXorAA;
typedef AsmXorEbxEbx AsmXorBB;
typedef AsmXorEcxEcx AsmXorCC;

typedef AsmTestEaxEax AsmTestAA;
typedef AsmTestEbxEbx AsmTestBB;
typedef AsmTestEcxEcx AsmTestCC;
typedef AsmTestEdxEdx AsmTestDD;
typedef AsmTestEsiEsi AsmTestSISI;
typedef AsmTestEdiEdi AsmTestDIDI;

typedef AsmIncEax AsmIncA;
typedef AsmIncEcx AsmIncC;
typedef AsmIncEdx AsmIncD;
typedef AsmIncEbx AsmIncB;
typedef AsmIncEsp AsmIncSP;
typedef AsmIncEbp AsmIncBP;
typedef AsmIncEsi AsmIncSI;
typedef AsmIncEdi AsmIncDI;
typedef AsmDecEax AsmDecA;
typedef AsmDecEcx AsmDecC;
typedef AsmDecEdx AsmDecD;
typedef AsmDecEbx AsmDecB;
typedef AsmDecEsp AsmDecSP;
typedef AsmDecEbp AsmDecBP;
typedef AsmDecEsi AsmDecSI;
typedef AsmDecEdi AsmDecDI;

typedef AsmMovEaxX AsmMovAX;
typedef AsmMovEcxX AsmMovCX;
typedef AsmMovEdxX AsmMovDX;
typedef AsmMovEbxX AsmMovBX;
typedef AsmMovEspX AsmMovSPX;
typedef AsmMovEbpX AsmMovBPX;
typedef AsmMovEsiX AsmMovSIX;
typedef AsmMovEdiX AsmMovDIX;

typedef AsmMovEbpEsp AsmMovBPSP;
typedef AsmMovEspEbp AsmMovSPBP;
typedef AsmMovEcxEax AsmMovCA;
typedef AsmMovEaxEcx AsmMovAC;
typedef AsmMovEaxEsp AsmMovASP;
typedef AsmMovEspEax AsmMovSPA;

typedef AsmMovEaxIEax AsmMovAIA;
typedef AsmMovEaxIEbx AsmMovAIB;
typedef AsmMovEbxIEbxX AsmMovBIBX;
typedef AsmMovEaxIEaxX AsmMovAIAX;
typedef AsmMovEaxIEspX AsmMovAISPX;
typedef AsmMovEcxIEspX AsmMovCISPX;
typedef AsmMovEdxIEspX AsmMovDISPX;
typedef AsmMovEbxIEspX AsmMovBISPX;
typedef AsmLeaEaxIEspX AsmLeaAISPX;
typedef AsmLeaEcxIEspX AsmLeaCISPX;
typedef AsmLeaEdxIEspX AsmLeaDISPX;
typedef AsmLeaEbxIEspX AsmLeaBISPX;

typedef AsmMovIEsp0X AsmMovISP0X;

typedef AsmMovIEspXEax AsmMovISPXA;
typedef AsmMovIEspEax AsmMovISPA;
typedef AsmMovIEspXEcx AsmMovISPXC;
typedef AsmMovIEspXEdx AsmMovISPXD;
typedef AsmMovIEspXEbx AsmMovISPXB;

typedef AsmMovIEbpXEax AsmMovIBPXA;
typedef AsmMovIEaxXEax AsmMovIAXA;

typedef AsmMovIXEax AsmMovIXA;
typedef AsmXchgEaxIEsp AsmXchgAISP;
typedef AsmXchgEcxIEsp AsmXchgCISP;
typedef AsmXchgEaxIEspX AsmXchgAISPX;
typedef AsmXchgEcxIEspX AsmXchgCISPX;
typedef AsmSubEaxX AsmSubAX;
typedef AsmSubEspX AsmSubSPX;
typedef AsmAddEspX AsmAddSPX;

typedef AsmJmpEax AsmJmpA;
typedef AsmJmpIEsp AsmJmpISP;
typedef AsmJmpIEspX AsmJmpISPX;
#endif

typedef AsmInstruction<2,0xd0ff> AsmCallEax;
typedef AsmInstruction<2,0xd1ff> AsmCallEcx;
typedef AsmInstruction<2,0xd2ff> AsmCallEdx;
typedef AsmInstruction<2,0xd3ff> AsmCallEbx;
typedef AsmInstruction<2,0xd4ff> AsmCallEsp;
typedef AsmInstruction<2,0xd5ff> AsmCallEbp;
typedef AsmInstruction<2,0xd6ff> AsmCallEsi;
typedef AsmInstruction<2,0xd7ff> AsmCallEdi;
typedef AsmInstruction<2,0x10ff> AsmCallIndirectEax;
typedef AsmInstruction<2,0x11ff> AsmCallIndirectEcx;
typedef AsmInstruction<2,0x12ff> AsmCallIndirectEdx;
typedef AsmInstruction<2,0x13ff> AsmCallIndirectEbx;
typedef AsmInstruction<2,0x14ff> AsmCallIndirectEsp;
typedef AsmInstruction<2,0x15ff> AsmCallIndirectEbp;
typedef AsmInstruction<2,0x16ff> AsmCallIndirectEsi;
typedef AsmInstruction<2,0x17ff> AsmCallIndirectEdi;

typedef AsmOperandInstruction<2,BYTE,0x4589> AsmMovIEbpXEax;
typedef AsmOperandInstruction<2,BYTE,0x4089> AsmMovIEaxXEax;
typedef AsmOperandInstruction<3,BYTE,0x244489,0> AsmMovIEspEax;
typedef AsmOperandInstruction<3,BYTE,0x244489> AsmMovEspXEax; //deprecated
typedef AsmOperandInstruction<3,BYTE,0x244489,4> AsmMovIEsp4Eax;
typedef AsmOperandInstruction<3,BYTE,0x244489,4> AsmMovEsp4Eax; //deprecated
typedef AsmOperandInstruction<3,BYTE,0x2404c6> AsmMovIEspB;
typedef AsmOperandInstruction<2,DWORD,0x1d8b> AsmMovEbxIX;
typedef AsmOperandInstruction<2,DWORD,0x0d8b> AsmMovEcxIX;

typedef AsmOperandInstruction<1,DWORD,0x68> AsmPushDword;

typedef Asm2OperandInstruction<2,DWORD,DWORD,0x05c7> AsmMovIXX;

typedef AsmInstruction<2,0x0089> AsmMovIEaxEax;
typedef AsmOperandInstruction<2,BYTE,0x01c6> AsmMovIEcxB;

typedef AsmInstruction<1,0xc3> AsmRet;
typedef AsmOperandInstruction<1,WORD,0xc2> AsmRetW;

typedef AsmOperandInstruction<2, BYTE,0xe883> AsmSubEaxB;

typedef AsmInstruction<1,0xcc> AsmInt3;

typedef AsmInstruction<3,0x2434ff> AsmPushIEsp;

typedef AsmRelativeOperandInstruction<1,0xeb, SBYTE> AsmNearJmp;
typedef AsmRelativeOperandInstruction<1,0xe9> AsmFarJmp;
typedef AsmRelativeOperandInstruction<1,0xe8> AsmFarCall;

#ifdef _WIN64
typedef struct _AsmPtrJmp
{
	AsmFarJmpIndirect m_Jmp;
	__unaligned LPCVOID mpvDest;
	typedef AsmFarJmpIndirect::_TInstruction _TInstruction;
	_AsmPtrJmp( __unaligned LPCVOID pvDest )
		: m_Jmp( (LPCVOID)&mpvDest )
		, mpvDest( pvDest )
		{
			if( !pvDest )
				Release();
		}
#pragma warning(push)
#pragma warning(disable : 4355)
	_AsmPtrJmp( const LONG_PTR lOffset )
		: m_Jmp( (LPCVOID)&mpvDest )
		, mpvDest( (const __unaligned BYTE*)this + sizeof(*this) + lOffset )
		{
		}
#pragma warning(pop)
	void Release() { FillMemory( this, sizeof(*this), 0x90 ); }
	void SetDestination( LPCVOID pvDestination )
		{
			new( this ) _AsmPtrJmp( pvDestination );
		}
	__unaligned LPCVOID GetDestination() const
		{
			if( CompareOpCodeBytes< AsmNop >( (const BYTE*)&m_Jmp ) )
				return NULL;
			if( !CompareOpCodeBytes< AsmFarJmpIndirect >( (const BYTE*)&m_Jmp ) )
				throw eBadFormat;
			return mpvDest;
		}
private:
	_AsmPtrJmp& operator=( const _AsmPtrJmp& ); //not allowed
} AsmPtrJmp;
typedef struct _AsmPtrCall
{
	AsmNearJmp m_SkipToCall;
	__unaligned LPCVOID mpvDest;
	AsmFarCallIndirect m_Call;
	typedef AsmNearJmp::_TInstruction _TInstruction;
	_AsmPtrCall( __unaligned LPCVOID pvDest )
		: m_SkipToCall( (LPCVOID)&m_Call ), mpvDest( pvDest ), m_Call( (LPCVOID)&mpvDest )
		{
			if( !pvDest )
				Release();
		}
#pragma warning(push)
#pragma warning(disable : 4355)
	_AsmPtrCall( const LONG_PTR lOffset )
		: m_SkipToCall( (LPCVOID)&m_Call )
		, mpvDest( (const __unaligned BYTE*)this + sizeof(*this) + lOffset )
		, m_Call( (LPCVOID)&mpvDest )
		{
		}
#pragma warning(pop)
	void Release() { FillMemory( this, sizeof(*this), 0x90 ); }
	void SetDestination( LPCVOID pvDestination )
		{
			new( this ) _AsmPtrCall( pvDestination );
		}
	__unaligned LPCVOID GetDestination() const
		{
			BYTE _Op = *(BYTE*)this;
			if( _Op == 0x90 )
				return NULL;
			if( _Op != (BYTE)AsmNearJmp::_OpCodes )
				throw eBadFormat;
			return mpvDest;
		}
private:
	_AsmPtrCall& operator=( const _AsmPtrCall& ); //not allowed
} AsmPtrCall;
typedef struct _AsmPushPtr
{
	AsmPushRax m_PushRax;
	AsmMovAX m_GetPtr;
	AsmXchgRaxIRsp m_StorePtr;
	typedef AsmPushRax::_TInstruction _TInstruction;
	_AsmPushPtr( __unaligned LPCVOID pvPtr ) : m_GetPtr( (DWORD_PTR)pvPtr ) {}
	_AsmPushPtr( DWORD_PTR pvPtr = 0 ) : m_GetPtr( pvPtr ) {}
	void Set( const DWORD_PTR opNew ) { m_GetPtr.Set( opNew ); }
	DWORD_PTR Get() const { return m_GetPtr.Get(); }
	static const size_t OffsetOfTargetAddress() { return (sizeof(AsmPushRax) + sizeof(AsmMovAX) - sizeof(DWORD_PTR)); }
} AsmPushPtr;
#else //_WIN64

typedef AsmFarJmp AsmPtrJmp;
typedef AsmFarCall AsmPtrCall;
#pragma warning(push)
#pragma warning(disable:4097)
struct AsmPushPtr : public AsmPushDword
{
	typedef AsmPushDword::_TInstruction _TInstruction;
	AsmPushPtr( DWORD dwOp = 0 ) : AsmPushDword( dwOp ) {}
	AsmPushPtr( LPCVOID  pvOp ) : AsmPushDword( (DWORD_PTR)pvOp ) {}
	static const size_t OffsetOfTargetAddress() { return sizeof(_TInstruction); }
};
#pragma warning(pop)

#endif //_WIN64

#pragma pack(pop)


//AsmRelativeOperandInstruction implementation
template<typename Ofs>
inline Ofs CalcOffset( LPCVOID pvTarget, LPCVOID pvBase )
{
	//if( (Ofs)-1 >= 0 )
	//	throw eBadFormat; //unsigned Ofs causes incorrect comparison of negative offset due to no sign extension
	ptrdiff_t lOffset = (ptrdiff_t)((BYTE*)pvTarget - (BYTE*)pvBase);
	ptrdiff_t const mask = lOffset >> (sizeof(ptrdiff_t) * 8 - 1);
	ptrdiff_t absOffset = (lOffset ^ mask) - mask;
	if( absOffset != (Ofs)absOffset )
		throw eBadFormat;
	return (Ofs)lOffset;
}

template<const unsigned int cbOpCodes, const DWORD_PTR Op, typename Ofs>
inline
AsmRelativeOperandInstruction<cbOpCodes,Op,Ofs>::AsmRelativeOperandInstruction( LPCVOID pvDestination )
#pragma warning(push)
#pragma warning(disable : 4355)
: AsmOperandInstruction<cbOpCodes,Ofs,Op>( CalcOffset<Ofs>( pvDestination, (const __unaligned BYTE*)this + sizeof(*this) ) )
#pragma warning(pop)
{
	if( !pvDestination )
		Release();
}

template<const unsigned int cbOpCodes, const DWORD_PTR Op, typename Ofs>
inline
AsmRelativeOperandInstruction<cbOpCodes,Op,Ofs>::AsmRelativeOperandInstruction( const Ofs dwOffset )
: AsmOperandInstruction<cbOpCodes,Ofs,Op>( dwOffset )
{}

template<const unsigned int cbOpCodes, const DWORD_PTR Op, typename Ofs>
inline
AsmRelativeOperandInstruction<cbOpCodes,Op,Ofs>::AsmRelativeOperandInstruction( const AsmNop& Nop )
: AsmOperandInstruction<cbOpCodes,Ofs,Op>( Nop )
{}

template<const unsigned int cbOpCodes, const DWORD_PTR Op, typename Ofs>
inline
void AsmRelativeOperandInstruction<cbOpCodes,Op,Ofs>::Release()
{
	FillMemory( this, sizeof(*this), 0x90 );
}

template<const unsigned int cbOpCodes, const DWORD_PTR Op, typename Ofs>
inline
void AsmRelativeOperandInstruction<cbOpCodes,Op,Ofs>::SetDestination( LPCVOID pvDestination )
{
	new( this ) AsmRelativeOperandInstruction<cbOpCodes,Op,Ofs>( pvDestination );
}

template<const unsigned int cbOpCodes, const DWORD_PTR Op, typename Ofs>
inline
__unaligned LPCVOID AsmRelativeOperandInstruction<cbOpCodes,Op,Ofs>::GetDestination() const
{
	BYTE _Op = *(BYTE*)this;
	if( _Op == 0x90 )
		return NULL;
	if( _Op != Op )
		throw eBadFormat;
	return (LPCVOID)((DWORD_PTR)this + Get() + sizeof(*this));
}

#endif //_ASM_H_
