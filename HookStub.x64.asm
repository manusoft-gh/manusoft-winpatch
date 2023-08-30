;// Copyright 2010 ManuSoft
;// https://www.manusoft.com
;// All Rights Reserved
;//
;// Use of this software is governed by the terms of:
;// GNU General Public License v2.0
;// See LICENSE for details

EXTERN ?CleanDestruct@HookContext@@SAXPEAD@Z : PROC
EXTERN PreserveRegsStdCall : PROC

PUBLIC ExitHookContext
PUBLIC InitHookContext
PUBLIC InsertArgs
PUBLIC InsertArgsThis

_TEXT SEGMENT 'CODE'

align 16
InitHookContext PROC

; Initialize HookContext structure
; rax = address of new HookContext

	lea rsp, [rax + 78h]
	push r13
	push r12
	push r11
	push r10
	push r9
	push r8
	pushfq
	push rdx
	push rcx
	push rbx
	push rax
	push rdi
	push rsi
	push rbp
	mov rbx, rax ;save hook context in rbx for use after hook function returns
	mov rsp, [rax + 78h]
	add rsp, 28h
	push r9
	push r8
	mov r9, rdx
	mov r8, rcx
	mov rdx, [rax + 80h]
	mov rcx, rax
	sub rsp, 20h
	mov rax, [rsp + 8h]
	jmp rax
InitHookContext ENDP

align 16
ExitHookContext PROC

; Unwind and delete HookContext structure
; rax = address of HookContext

	mov rbx, [rax + 78h]
	mov [rbx - 8h], rax
	lea rsp, [rax + 8h]
	mov rax, [rax + 80h]
	mov [rbx - 20h], rax
	mov rax, [rsp - 8h]
	mov [rbx], rax
	pop rbp
	pop rsi
	pop rdi
	pop rax
	pop rbx
	pop rcx
	pop rdx
	popfq
	pop r8
	pop r9
	pop r10
	pop r11
	pop r12
	pop r13
	pop rsp
	sub rsp, 20h
	mov qword ptr [rsp + 10h], 8h
	mov [rsp + 8h], rax
	mov rax, ?CleanDestruct@HookContext@@SAXPEAD@Z
	xchg rax, [rsp + 8h]
	jmp PreserveRegsStdCall
ExitHookContext ENDP

align 16
InsertArgs PROC

; shift existing arguments and insert new arguments
; [rsp+0h] = hook state argument to insert (omit if NULL)
; [rsp+8h] = original function address argument to insert
; [rsp+10h] = hook function address
; [rsp+18h] = number of arguments
; [rsp+20h] = original return address

	push rbp
	mov rbp, rsp
	sub rsp, 30h
	mov rax, [rbp + 20h]
	cmp rax, 4
	jl ShiftRegArgs
	mov r10, rax
	or r10, 1
	inc r10
	shl r10, 3
	sub r10, 20h
	sub rsp, r10
	push rdi
	push rsi
	sub rax, 3
	lea rsi, [rbp + 50h]
	lea rdi, [rsp + 30h]
	cmp qword ptr [rbp + 8h], 0
	jz MoveStackArgs
	add rdi, 8h
MoveStackArgs:
	dec rax
	jz ShiftAllRegArgs
	add rdi, 8h
	add rsi, 8h
	mov r10, [rsi]
	mov [rdi], r10
	jmp MoveStackArgs
ShiftAllRegArgs:
	pop rsi
	pop rdi
	cmp qword ptr [rbp + 8h], 0
	jz ShiftAll1
	mov [rsp + 28], r9
	mov [rsp + 20], r8
	mov r9, rdx
	mov r8, rcx
	mov rdx, [rbp + 10h]
	mov rcx, [rbp + 8h]
	jmp CallHook
ShiftAll1:
	mov [rsp + 20], r9
	mov r9, r8
	mov r8, rdx
	mov rdx, rcx
	mov rcx, [rbp + 10h]
	jmp CallHook
	
ShiftRegArgs:
	cmp qword ptr [rbp + 8h], 0
	jz Shift1
	cmp rax, 4
	jl Arg3Shift2
	mov [rsp + 28h], r9
Arg3Shift2:
	cmp rax, 3
	jl Arg2Shift2
	mov [rsp + 20h], r8
Arg2Shift2:
	cmp rax, 2
	jl Arg1Shift2
	mov r9, rdx
Arg1Shift2:
	cmp rax, 1
	jl CallHook
	mov r8, rcx
	mov rdx, [rbp + 10h]
	mov rcx, [rbp + 8h]
	jmp CallHook
Shift1:
	cmp rax, 4
	jl Arg3Shift1
	mov [rsp + 20h], r9
Arg3Shift1:
	cmp rax, 3
	jl Arg2Shift1
	mov r9, r8
Arg2Shift1:
	cmp rax, 2
	jl Arg1Shift1
	mov r8, rdx
Arg1Shift1:
	cmp rax, 1
	jl CallHook
	mov rdx, rcx
	mov rcx, [rbp + 10h]

CallHook:
	call qword ptr [rbp + 18h]
	mov rsp, rbp
	pop rbp
	add rsp, 20h
	ret
InsertArgs ENDP

align 16
InsertArgsThis PROC

; shift existing arguments and insert new arguments
; rcx = "this" pointer
; [rsp+0h] = hook state argument to insert (omit if NULL)
; [rsp+8h] = original function address argument to insert
; [rsp+10h] = hook function address
; [rsp+18h] = number of arguments
; [rsp+20h] = original return address

	push rbp
	mov rbp, rsp
	sub rsp, 30h
	mov rax, [rbp + 20h]
	cmp rax, 4
	jl ShiftRegArgs
	mov r10, rax
	or r10, 1
	inc r10
	shl r10, 3
	sub r10, 20h
	sub rsp, r10
	push rdi
	push rsi
	sub rax, 3
	lea rsi, [rbp + 50h]
	lea rdi, [rsp + 30h]
	cmp qword ptr [rbp + 8h], 0
	jz MoveStackArgs
	add rdi, 8h
MoveStackArgs:
	dec rax
	jz ShiftAllRegArgs
	add rdi, 8h
	add rsi, 8h
	mov r10, [rsi]
	mov [rdi], r10
	jmp MoveStackArgs
ShiftAllRegArgs:
	pop rsi
	pop rdi
	cmp qword ptr [rbp + 8h], 0
	jz ShiftAll1
	mov [rsp + 28], r9
	mov [rsp + 20], r8
	mov r9, rdx
	mov r8, [rbp + 10h]
	mov rdx, rcx
	mov rcx, [rbp + 8h]
	jmp CallHook
ShiftAll1:
	mov [rsp + 20], r9
	mov r9, r8
	mov r8, rdx
	mov rdx, [rbp + 10h]
	jmp CallHook
	
ShiftRegArgs:
	cmp qword ptr [rbp + 8h], 0
	jz Shift1
	cmp rax, 4
	jl Arg3Shift2
	mov [rsp + 28h], r9
Arg3Shift2:
	cmp rax, 3
	jl Arg2Shift2
	mov [rsp + 20h], r8
Arg2Shift2:
	cmp rax, 2
	jl Arg1Shift2
	mov r9, rdx
Arg1Shift2:
	cmp rax, 1
	jl CallHook
	mov rdx, [rbp + 10h]
	mov r8, rcx
	mov rcx, [rbp + 8h]
	jmp CallHook
Shift1:
	cmp rax, 4
	jl Arg3Shift1
	mov [rsp + 20h], r9
Arg3Shift1:
	cmp rax, 3
	jl Arg2Shift1
	mov r9, r8
Arg2Shift1:
	cmp rax, 2
	jl Arg1Shift1
	mov r8, rdx
Arg1Shift1:
	cmp rax, 1
	jl CallHook
	mov rdx, rcx
	mov rcx, [rbp + 10h]

CallHook:
	call qword ptr [rbp + 18h]
	mov rsp, rbp
	pop rbp
	add rsp, 20h
	ret
InsertArgsThis ENDP

_TEXT ENDS

END
