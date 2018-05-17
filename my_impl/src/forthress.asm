%include "lib.inc"
%include "macro.inc"

%define pc r15
%define w r14
%define rstack r13

%include "words.inc"

section .bss

; return stack
resq 1023
rstack_start: resq 1
;

input_buf: resb 1024
user_dict: resq 65536

user_mem: resq 65536

state: resq 1

section .data
last_word: dq previous_word
here: dq user_dict
dp: dq user_mem

section .rodata
msg_no_such_word: db ": no such word", 10, 0

section .text
next:
	mov w, pc
	add pc, 8
	mov w, [w]
	jmp [w]
	
global _start
_start:	
	jmp i_init
