// Copyright 2010 ManuSoft
// https://www.manusoft.com
// All Rights Reserved
//
// Use of this software is governed by the terms of:
// GNU General Public License v2.0
// See LICENSE for details

#include "StdAfx.h"
#include "PeVersion.h"

#pragma comment (lib, "version.lib")


void PeFileVersion::InitializeA( LPCSTR pszModulePath )
{
	CHAR szModulePath[MAX_PATH];
	lstrcpynA( szModulePath, pszModulePath, MAX_PATH );
	DWORD dwHandle;
	DWORD dwVerSize = GetFileVersionInfoSizeA( szModulePath, &dwHandle );
	if( dwVerSize == 0 )
		return;

	LPVOID pvBuffer = new BYTE[dwVerSize];
	if( GetFileVersionInfoA( szModulePath, dwHandle, dwVerSize, pvBuffer ) )
	{
		VS_FIXEDFILEINFO*	pFixedInfo;
		UINT nSize;
		if( VerQueryValueA( pvBuffer, "\\", reinterpret_cast< void** >(&pFixedInfo), &nSize ) )
		{
			DWORD dwVersionMS = pFixedInfo->dwFileVersionMS;
			DWORD dwVersionLS = pFixedInfo->dwFileVersionLS;
			m_wMajor = HIWORD(dwVersionMS);
			m_wMinor = LOWORD(dwVersionMS);
			m_wThird = HIWORD(dwVersionLS);
			m_wFourth = LOWORD(dwVersionLS);
		}
	}
	delete[] pvBuffer;
}

void PeFileVersion::InitializeW( LPCWSTR pszModulePath )
{
	WCHAR szModulePath[MAX_PATH];
	lstrcpynW( szModulePath, pszModulePath, MAX_PATH );
	DWORD dwHandle;
	DWORD dwVerSize = GetFileVersionInfoSizeW( szModulePath, &dwHandle );
	if( dwVerSize == 0 )
		return;

	LPVOID pvBuffer = new BYTE[dwVerSize];
	if( GetFileVersionInfoW( szModulePath, dwHandle, dwVerSize, pvBuffer ) )
	{
		VS_FIXEDFILEINFO*	pFixedInfo;
		UINT nSize;
		if( VerQueryValueW( pvBuffer, L"\\", reinterpret_cast< void** >(&pFixedInfo), &nSize ) )
		{
			DWORD dwVersionMS = pFixedInfo->dwFileVersionMS;
			DWORD dwVersionLS = pFixedInfo->dwFileVersionLS;
			m_wMajor = HIWORD(dwVersionMS);
			m_wMinor = LOWORD(dwVersionMS);
			m_wThird = HIWORD(dwVersionLS);
			m_wFourth = LOWORD(dwVersionLS);
		}
	}
	delete[] pvBuffer;
}
