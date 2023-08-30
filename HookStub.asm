;// Copyright 2010 ManuSoft
;// https://www.manusoft.com
;// All Rights Reserved
;//
;// Use of this software is governed by the terms of:
;// GNU General Public License v2.0
;// See LICENSE for details

.386

EXTERN ?CleanDestruct@HookContext@@SGXPAD@Z : PROC
EXTERN _PreserveRegsStdCall : PROC

PUBLIC _InitHookContext
PUBLIC _ExitHookContext

_TEXT SEGMENT 'CODE'

align 16
_InitHookContext PROC

; Initialize HookContext structure
; eax = address of new HookContext

	lea esp, [eax + 24h]
	pushfd
	push edx
	push ecx
	push ebx
	push eax
	push edi
	push esi
	push ebp
	mov ebx, eax ;save hook context in ebx for use after hook function returns
	mov esp, [eax + 24h]
	mov eax, [ebx + 28h]
	xchg eax, [esp]
	push ebx
	jmp eax
_InitHookContext ENDP

align 16
_ExitHookContext PROC

; Unwind and delete HookContext structure
; eax = address of HookContext

	mov ebx, [eax + 24h]
	mov [ebx - 4h], eax
	lea esp, [eax + 4h]
	mov eax, [eax + 28h]
	mov [ebx - 10h], eax
	mov eax, [esp - 4h]
	mov [ebx], eax
	pop ebp
	pop esi
	pop edi
	pop eax
	pop ebx
	pop ecx
	pop edx
	popfd
	pop esp
	sub esp, 4h
	push 4h
	mov eax, ?CleanDestruct@HookContext@@SGXPAD@Z
	push eax
	sub esp, 4h
	jmp _PreserveRegsStdCall
_ExitHookContext ENDP

_TEXT ENDS

END
