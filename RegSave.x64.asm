;// Copyright 2010 ManuSoft
;// https://www.manusoft.com
;// All Rights Reserved
;//
;// Use of this software is governed by the terms of:
;// GNU General Public License v2.0
;// See LICENSE for details

PUBLIC PreserveRegsStdCall
PUBLIC PreserveRegsCDecl
PUBLIC PreserveRegsStdCallRet
PUBLIC PreserveRegsCDeclRet

_TEXT SEGMENT 'CODE'


; typedef void (__stdcall *F_StdCallTargetFunction) ( va_list args );
; extern void __stdcall PreserveRegsStdCall( F_StdCallTargetFunction pfTarget, DWORD_PTR cbArgs, ... );

align 16
PreserveRegsStdCall PROC
	push rbp                       ; set up stack frame
	mov rbp, rsp
; align stack here
	sub rsp, 8h
	or rsp, 8h
; [rbp + 8]    return address
; [rbp + 10h]  address of target function
; [rbp + 18h]  size (in bytes) of argument list
; [rbp + 20h]  beginning of stack arguments
	push rax                       ; save registers
	push rbx
	push rcx
	push rdx
	push r8
	push r9
	push r10
	push r11
	pushfq                         ; save flags
	sub rsp, 20h                   ; allocate shadow space
	lea rcx, [rbp + 20h]           ; get vararg list
	mov rax, qword ptr [rbp + 10h] ; get target function
	call rax                       ; and call it
	add rsp, 20h                   ; deallocate shadow space
	mov rdx, qword ptr [rbp + 8]   ; get return address
	mov rbx, [rbp + 18h]           ; get vararg list size (must be multiple of 10h!)
	mov qword ptr [rbp + rbx + 18h], rdx ; move return address to top of arg stack space
	mov qword ptr [rbp + 8], rax   ; stash the return value in case the caller wants it
	popfq                          ; restore flags
	pop r11                        ; restore registers
	pop r10
	pop r9
	pop r8
	pop rdx
	pop rcx
	pop rbx
	pop rax
; re-align stack here
	mov rsp, rbp
	pop rbp                        ; clean up stack frame
	add rsp, 10h                   ; deallocate fixed arguments
	add rsp, qword ptr [rsp]       ; deallocate vararg arguments
	ret
PreserveRegsStdCall ENDP


; typedef void (__cdecl *F_CDeclTargetFunction) ( va_list args );
; extern void __stdcall PreserveRegsCDecl( F_CDeclTargetFunction pfTarget, DWORD_PTR cbArgs, ... );

align 16
PreserveRegsCDecl PROC
	jmp PreserveRegsStdCall
PreserveRegsCDecl ENDP


; typedef DWORD_PTR (__stdcall *F_StdCallTargetFunctionRet) ( va_list args );
; extern DWORD_PTR __stdcall PreserveRegsStdCallRet( F_StdCallTargetFunctionRet pfTarget, DWORD_PTR cbArgs, ... );

align 16
PreserveRegsStdCallRet PROC
	push rbp                       ; set up stack frame
	mov rbp, rsp
; align stack here
	and rsp, 0FFFFFFFFFFFFFFF7h
	push rbx                       ; save registers
	push rcx
	push rdx
	push r8
	push r9
	push r10
	push r11
	pushfq                         ; save flags
	sub rsp, 20h                   ; allocate shadow space
	lea rcx, [rbp + 20h]           ; get vararg list
	mov rax, qword ptr [rbp + 10h] ; get target function
	call rax                       ; and call it
	add rsp, 20h                   ; deallocate shadow space
	mov rdx, qword ptr [rbp + 8]   ; get return address
	mov rbx, [rbp + 18h]           ; get vararg list size (must be multiple of 10h!)
	mov qword ptr [rbp + rbx + 18h], rdx ; move return address to top of arg stack space
	mov qword ptr [rbp + 8], rax   ; stash the return value in case the caller wants it
	popfq                          ; restore flags
	pop r11                        ; restore registers
	pop r10
	pop r9
	pop r8
	pop rdx
	pop rcx
	pop rbx
; re-align stack here
	mov rsp, rbp
	pop rbp                        ; clean up stack frame
	add rsp, 10h                   ; deallocate fixed arguments
	add rsp, qword ptr [rsp]       ; deallocate vararg arguments
	ret
PreserveRegsStdCallRet ENDP


; typedef DWORD_PTR (__cdecl *F_CDeclTargetFunctionRet) ( va_list args );
; extern DWORD_PTR __stdcall PreserveRegsCDeclRet( F_CDeclTargetFunctionRet pfTarget, DWORD_PTR cbArgs, ... );

align 16
PreserveRegsCDeclRet PROC
	jmp PreserveRegsStdCallRet
PreserveRegsCDeclRet ENDP

_TEXT ENDS

END
