;// Copyright 2010 ManuSoft
;// https://www.manusoft.com
;// All Rights Reserved
;//
;// Use of this software is governed by the terms of:
;// GNU General Public License v2.0
;// See LICENSE for details

.386

PUBLIC _PreserveRegsStdCall
PUBLIC _PreserveRegsCDecl
PUBLIC _PreserveRegsStdCallRet
PUBLIC _PreserveRegsCDeclRet

_TEXT SEGMENT 'CODE'

; typedef void (__stdcall *F_StdCallTargetFunction) ( va_list args );
; void __stdcall PreserveRegsStdCall( F_StdCallTargetFunction pfTarget, DWORD_PTR cbArgs, ... )

align 16
_PreserveRegsStdCall PROC

	push ebp                       ; set up stack frame
	mov ebp, esp
	push eax                       ; save registers
	push ebx
	push ecx
	push edx
	push esi
	push edi
	pushfd                         ; save flags
	lea eax, [ebp + 10h]           ; get address of arg list
	push eax                       ; pass vararg list to target function
	mov eax, dword ptr [ebp + 8]   ; get target function
	call eax                       ; and call it
	mov edx, dword ptr [ebp + 4]   ; get return address
	mov ebx, dword ptr [ebp + 0Ch] ; get vararg list size
	mov dword ptr [ebp + ebx + 0Ch], edx ; move return address to top of arg stack space
	popfd                          ; restore flags
	pop edi                        ; restore registers
	pop esi
	pop edx
	pop ecx
	pop ebx
	pop eax
	pop ebp                        ; clean up stack frame
	add esp, 8                     ; deallocate fixed arguments
	add esp, dword ptr [esp]       ; deallocate vararg arguments
	ret

_PreserveRegsStdCall ENDP


; typedef void (__cdecl *F_CDeclTargetFunction) ( va_list args );
; void __stdcall PreserveRegsCDecl( F_CDeclTargetFunction pfTarget, DWORD_PTR cbArgs, ... )

align 16
_PreserveRegsCDecl PROC

	push ebp                       ; set up stack frame
	mov ebp, esp
	push eax                       ; save registers
	push ebx
	push ecx
	push edx
	push esi
	push edi
	pushfd                         ; save flags
	lea eax, [ebp + 10h]           ; get address of arg list
	push eax                       ; pass vararg list to target function
	mov eax, dword ptr [ebp + 8]   ; get target function
	call eax                       ; and call it
	pop edx                        ; remove arg from stack
	mov edx, dword ptr [ebp + 4]   ; get return address
	mov ebx, dword ptr [ebp + 0Ch] ; get vararg list size
	mov dword ptr [ebp + ebx + 0Ch], edx ; move return address to top of arg stack space
	popfd                          ; restore flags
	pop edi                        ; restore registers
	pop esi
	pop edx
	pop ecx
	pop ebx
	pop eax
	pop ebp                        ; clean up stack frame
	add esp, 8                     ; deallocate fixed arguments
	add esp, dword ptr [esp]       ; deallocate vararg arguments
	ret

_PreserveRegsCDecl ENDP


; typedef DWORD_PTR (__stdcall *F_StdCallTargetFunctionRet) ( va_list args );
; DWORD_PTR __stdcall PreserveRegsStdCallRet( F_StdCallTargetFunction pfTarget, DWORD_PTR cbArgs, ... )

align 16
_PreserveRegsStdCallRet PROC

	push ebp                       ; set up stack frame
	mov ebp, esp
	push ebx                       ; save registers
	push ecx
	push edx
	push esi
	push edi
	pushfd                         ; save flags
	lea eax, [ebp + 10h]           ; get address of arg list
	push eax                       ; pass vararg list to target function
	mov eax, dword ptr [ebp + 8]   ; get target function
	call eax                       ; and call it
	mov edx, dword ptr [ebp + 4]   ; get return address
	mov ebx, dword ptr [ebp + 0Ch] ; get vararg list size
	mov dword ptr [ebp + ebx + 0Ch], edx ; move return address to top of arg stack space
	popfd                          ; restore flags
	pop edi                        ; restore registers
	pop esi
	pop edx
	pop ecx
	pop ebx
	pop ebp                        ; clean up stack frame
	add esp, 8                     ; deallocate fixed arguments
	add esp, dword ptr [esp]       ; deallocate vararg arguments
	ret

_PreserveRegsStdCallRet ENDP


; typedef DWORD_PTR (__cdecl *F_CDeclTargetFunctionRet) ( va_list args );
; DWORD_PTR __stdcall PreserveRegsCDeclRet( F_CDeclTargetFunction pfTarget, DWORD_PTR cbArgs, ... )

align 16
_PreserveRegsCDeclRet PROC

	push ebp                       ; set up stack frame
	mov ebp, esp
	push ebx                       ; save registers
	push ecx
	push edx
	push esi
	push edi
	pushfd                         ; save flags
	lea eax, [ebp + 10h]           ; get address of arg list
	push eax                       ; pass vararg list to target function
	mov eax, dword ptr [ebp + 8]   ; get target function
	call eax                       ; and call it
	pop edx                        ; remove arg from stack
	mov edx, dword ptr [ebp + 4]   ; get return address
	mov ebx, dword ptr [ebp + 0Ch] ; get vararg list size
	mov dword ptr [ebp + ebx + 0Ch], edx ; move return address to top of arg stack space
	popfd                          ; restore flags
	pop edi                        ; restore registers
	pop esi
	pop edx
	pop ecx
	pop ebx
	pop ebp                        ; clean up stack frame
	add esp, 8                     ; deallocate fixed arguments
	add esp, dword ptr [esp]       ; deallocate vararg arguments
	ret

_PreserveRegsCDeclRet ENDP

_TEXT ENDS

END
