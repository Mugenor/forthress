; rot 
section .data
	stack_base: dq 0
; Shows all stack contents. Does not pop elements
native ".S", show_stack
    mov rcx, rsp
    .loop:
        cmp rcx, [stack_base]
        jae next
        mov rdi, [rcx]
        push rcx
        call print_int
        call print_newline
        pop rcx
        add rcx, 8
        jmp .loop

native ".", dot
	pop rdi
	call print_int
	call print_newline
	jmp next

; Read word and place its address on stack
; ( -- a )
colon "'", tick, 1
	dq xt_inbuf, xt_word, xt_drop
	dq xt_inbuf, xt_find, xt_dup
	branch0 .no_word
	dq xt_cfa
	dq xt_state, xt_fetch
	branch0 .interpret
	dq xt_lit, xt_lit
	dq xt_comma, xt_comma ; sequence of commands xt_lit, xt
.interpret:
	dq xt_exit
.no_word:
	dq xt_inbuf, xt_prints
	dq xt_lit, msg_no_such_word, xt_prints
	dq xt_exit

; output one character from TOS
native "emit", emit
	pop rdi 
	call print_char
	jmp next
	
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
	dq xt_lit, in_fd, xt_fetch, xt_to_r ; data ( stdin ) return ( in_fd )
	dq xt_lit, in_fd 					; data ( in_fd stdin ) return ( in_fd )
	dq xt_write							; data (  ) return ( in_fd )
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

; Main forthress interpretater
colon "interpret", interpret 
	dq xt_find, xt_dup ; ( word_headerX2 )
	branch0	.not_word
	dq xt_cfa
	dq xt_state, xt_fetch
	branch0 .interpret
	dq xt_dup
	; ( xt xt )
	; check for IMMEDIATE
	dq xt_lit, 1, xt_minus, xt_fetch_char, xt_not ; ( flag xt )
	branch0 .interpret		; if flag=0 then interpret word
	dq xt_comma
	dq xt_exit

.interpret:	
	dq xt_execute
	dq xt_exit
.not_word:
	; ( word_header )
	dq xt_drop
	dq xt_inbuf
	dq xt_number ; ( int len )
	dq xt_swap
	branch0 .error
	; ( int )
	dq xt_state, xt_fetch
	branch0 .number_interpret
	dq xt_here, xt_lit, 8, xt_minus, xt_fetch, xt_dup 
	; ( prev_xt prev_xt int )
	dq xt_lit, xt_branch, xt_equals
	; ( res prev_xt int )
	dq xt_swap, xt_lit, xt_branch0, xt_equals
	dq xt_lor
	branch0 .lit_numb
	dq xt_comma, xt_exit
	
.lit_numb:
	; ( int )
	dq xt_lit, xt_lit, xt_comma, xt_comma
	dq xt_exit
	
.number_interpret:
	dq xt_exit
.error:
	dq xt_inbuf, xt_prints
	dq xt_lit, msg_no_such_word, xt_prints
	dq xt_exit
	
	
; ( xt -- )
; Execute word with this execution token
native "execute", execute
	pop rax
	mov w, rax
	jmp [rax]


	
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
	
native "r@", r_fetch
	push qword[rstack]
	jmp next

native "land", land
	pop rax
	pop rdx
	test  rax, rax
	jz .zero
	push rdx
	jmp next
.zero:
	push rax
	jmp next
	
native "lor", lor
	pop rax
	pop rdx
	test rax, rax
	jnz .not_zero
	push rdx
	jmp next
.not_zero:
	push rax
	jmp next
	
native "and", and
	pop rax
	and [rsp], rax
	jmp next
	
native "or", or
	pop rax
	or [rsp], rax
	jmp next

; (addr val -- )
; Store value by address
native "!", write
	pop rax
	pop rdx
	mov [rax], rdx
	jmp next
	
; Stores a single char
; (addr char -- )
native "c!", write_char
	pop rax
	pop rdx
	mov [rax], dl
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
; (str -- header_addr)
native "find", find
	mov rsi, [last_word]
	
	mov rax, [state]
	test rax, rax
	jz .loop
	mov rsi, [rsi]	; in compiling mode skipping first word
.loop:
	mov rdi, [rsp]
	push rsi	
	add rsi, 8
	call string_equals
	pop rsi
	test rax, rax
	jnz .found
	
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
	jz .end
	inc rax
	jmp .loop
.end:
	add rax, 2
	push rax
	jmp next



;(addr -- value)
native "@", fetch
	pop rax
	push qword[rax]
	jmp next
	
native "c@", fetch_char
	pop rax
	movzx rax, byte[rax]
	push rax
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
	
	
native "+", plus
	pop rax
	add [rsp], rax
	jmp next
	
native "*", mul
	pop rax
	pop rdx
	imul rdx
	push rax
	jmp next
	
native "/", div
	pop rcx
	pop rax
	cqo
	idiv rcx
	push rax
	jmp next
	
native "%", mod
	pop rcx
	pop rax
	cqo
	idiv rcx
	push rdx
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
	
native "rot", rot
	pop rax
	pop rdx
	pop rdi
	push rax
	push rdi
	push rdx
	jmp next
	
native "drop", drop
	add rsp, 8
	jmp next

; ( x -- )
; Add x to the word being defined
native ",", comma
	mov rax, [here]
	pop qword[rax]
	add qword[here], 8
	jmp next
	
native "c,", char_comma
	mov rax, [here]
	pop rdx
	mov [rax], dl
	add qword[here], 1
	jmp next

;exit from colon word
native "exit", exit
	rpop pc
	jmp next
	

;( str -- int len)	
native "number", number
	pop rdi
	call parse_int
	push rdx
	push rax
	jmp next

; pushes state value on stack (0 - interpretation, 1 - compilation)	
const state, state
const inbuf, input_buf
const here, [here]
const last_word, last_word
const mem, user_mem
const dp, dp
const in_fd, in_fd

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

native "<", lower
	pop rdx
	pop rax
	cmp rax, rdx
	setl al
	movzx rax, al
	push rax
	jmp next



native "prints", prints
	pop rdi
	call print_string
	jmp next
	
colon "bye", bye
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
	call string_length
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
	
; ( str cnt -- )
; Prints cnt chars from str
native "printc", printc
	mov rax, 1
	mov rdi, 1
	pop rsi
	pop rdx
	syscall
	jmp next

; Copy a string to a buffer
; ( dst src -- )
native "str_copy", str_copy
	pop rsi
	pop rdi
	call string_copy
	jmp next

; ( name flags --  )
; Create an entry in the dictionary
; name is the new name as a null-terminated string
; only immediate flag is implemented ATM	
native "create", create
	mov rsi, [here]
	mov rcx, [last_word]
	mov [rsi], rcx
	mov [last_word], rsi
	add rsi, 8
	
	pop rdi
	push rsi
	call string_copy
	pop rdi
	push rdi
	call string_length
	pop rsi
	add rsi, rax
	inc rsi
	pop rax
	mov [rsi], al
	inc rsi
	
	mov [here], rsi
	jmp next
	
	
; Read word from stdin and start defining it

colon ":", colon
.again:
	dq xt_inbuf, xt_word
	branch0 .again
	dq xt_lit, 0, xt_inbuf, xt_create
	dq xt_lit, i_docol, xt_comma
	dq xt_lit, 1, xt_state, xt_write
	dq xt_exit

; End the current word definition	
colon ";", semicolon, 1
	dq xt_lit, xt_exit, xt_comma
	dq xt_lit, 0, xt_state, xt_write
	dq xt_exit