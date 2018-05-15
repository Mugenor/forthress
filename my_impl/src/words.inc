section .data
	stack_base: dq 0

native "init", init
	mov qword[state], 0
	mov rstack, rstack_start
	mov pc, interpreter_stub
	cmp qword[stack_base], 0
	je .first
	mov rsp, [stack_base] ; if init called not first time return rsp to stack_base
	jmp next
.first:
	mov [stack_base], rsp ; if init called first time
	jmp next

section .rodata
interpreter_stub:
	dq xt_lit, 0	;stdin
	dq xt_interpret_fd
	dq xt_bye
	
;reads words from an open fd and interprets them
;(fd -- )
colon "interpret_fd", interpret_fd
	dq xt_lit, in_fd, xt_fetch, xt_to_r
	dq xt_lit, in_fd
	dq xt_write
	.loop:
	dq xt_inbuf, xt_word
	branch0 .end_of_input
	dq xt_inbuf
	dq xt_interpret
	
	branch .loop
	
.end_of_input:
	dq xt_from_r
	dq xt_lit, in_fd, xt_write
	dq xt_exit
	
colon "interpret", interpret 
	
	
	
;pop from data stack to return stack
native ">r", to_r
	pop rax
	rpush rax
	jmp next

; Push from return stack into data stack
native "r>", from_r
	rpop rax
	push rax
	jmp next

; (addr val -- )
; Store value by address
native "!", write
	pop rax
	pop rdx
	mov [rax], rdx
	jmp next
	
;(addr -- len)
;Reads word from stdin and stores it starting at address
;Word length is pushed into stack
native "word", word
	pop rdi
	call read_word
	push rdx
	jmp next
	
native "dup", dup
	push qword[rsp]
	jmp next

; accepts pointer to a str, returns pointer to the word header in dictionary
; (str -- header addr)
native "find", find
	mov rsi, [last_word]
	
	;FOR COMPILING
	mov rax, [state]
	test rax, rax
	jz .loop
	mov rsi, [rsi]
.loop:
	mov rdi, [rsp]
	push rsi	
	add rsi, 8
	call string_equals
	pop rsi
	test al, al
	jz .found
	
	mov rsi, [rsi]
	test rsi, rsi
	jnz .loop ;NOT FOUND ?
.found:
	mov [rsp], rsi
	jmp next
	

; Jump to a location if TOS = 0. Location is calculated in a similar way
; 0branch is a compile-only word.
native "0branch", branch0
	pop rax 
	test rax, rax
	jnz .end
	mov pc, [pc]
	jmp next	
.end:
	add pc, 8
	jmp next

; Jump to a location. Location is an absolute address
; Branch is a compile-only word.
native "branch", branch
	mov pc, [pc]
	jmp next

;( word_addr -- xt )
native "cfa", cfa
	pop rax
	add rax, 8
.loop:
	mov sil, [rax]
	test sil, sil
	jz .loop
	inc rax
	jmp .loop
.end:
	add rax, 2
	push rax
	jmp next

; pushes state value on stack (0 - interpretation, 1 - compilation)	
const state, state
const inbuf, input_buf

;(addr -- value)
native "@", fetch
	pop rax
	push qword[rax]
	jmp next

; pushes current value to datastack
native "lit", lit
	push qword[pc]
	add pc, 8
	jmp next
	
native "-", minus
	pop rax
	sub [rsp], rax
	jmp next
	
;(addr -- char)
native "c@", fetch_char
	pop rax
	movzx rax, byte[rax]
	push rax
	jmp next
	
;(0 -- 1)
;(1 -- 0)
native "not", not
	pop rax
	test rax, rax
	setz al
	movzx rax, al
	push rax
	jmp next

native "swap", swap
	pop rax
	pop rdi
	push rax
	push rdi
	jmp next
	
native "drop", drop
	add rsp, 8
	jmp next

;???
native ",", comma
	mov rax, [here]
	pop qword[rax]
	add qword[here], 8
	jmp next

;exit from colon word
native "exit", exit
	rpop pc
	jmp next
	
;(xt --)
;Execute word with XT on TOS
native "execute", execute
	pop rax
	mov w, rax
	jmp [rax]

;( str -- int len)	
native "number", number
	pop rdi
	call parse_int
	push rax
	push rdx
	jmp next

const here, [here]

;( a b -- c)
;if (a==b) c=1; else c=0;
native "=", equals
	pop rax
	pop rdi
	cmp rax, rdi
	sete al
	movzx rax, al
	push rax
	jmp next

colon "prints", prints
	pop rdi
	call print_string
	jmp next
	
native "bye", bye
	dq xt_lit, 60
	dq xt_lit, 0
	dq xt_lit, 0
	dq xt_lit, 0
	dq xt_lit, 0
	dq xt_lit, 0
	dq xt_lit, 0
	dq xt_syscall
   
native "count", count
	pop rdi
	call strind_length
	push rax
	jmp next

native "docol", docol
	rpush pc
	add w, 8
	mov pc, w
	jmp next

; ( call_num a1 a2 a3 a4 a5 a6 -- new_rax new_rdx )
native "syscall", syscall
	pop r9
	pop r8
	pop r10
	pop rdx
	pop rsi
	pop rdi
	pop rax
	syscall
	push rdx
	push rax
	jmp next

