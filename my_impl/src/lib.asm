section .text
global string_length:
string_length:
	xor rax, rax
	.loop:
		cmp byte[rdi + rax], 0 ;пока не ноль, увеличь счетчик на 1
		je .end
		inc rax
		jmp .loop	
	.end:
    ret

global print_string:
print_string:
    xor rax, rax
    call string_length    		; узнать длину строки и вывести её
    mov rdx, rax
    mov rax, 1
    mov rsi, rdi
    mov rdi, 1
    syscall
		
    ret
    
global print_error:
print_error:
	xor rax, rax
	call string_length
	mov rdx, rax
	mov rax, 1
	mov rsi, rdi
	mov rdi, 2
	syscall
	ret


global print_char:
print_char:
    xor rax, rax
    mov rdx, 1
    mov rax, 1
    push rdi
    mov rsi, rsp
    mov rdi, 1
    syscall
    pop rdi
    
    ret

global print_newline:
print_newline:
    xor rax, rax
    mov rdi, 10
    call print_char;
    
    ret


global print_uint:
print_uint:
    mov r8, rsp
    dec rsp
    mov byte[rsp], 0
    mov rax, rdi
    mov r9, 10
    .loop:
		xor rdx, rdx
		div r9
		add rdx, '0'
		dec rsp
		mov byte[rsp], dl
		test rax, rax
		jnz .loop
	mov rdi, rsp
	push r8
	call print_string
	pop r8
	mov rsp, r8
	xor rax, rax
    ret


global print_int:
print_int:
	mov rax, rdi
	shl rax, 1
	mov rax, rdi
	jnc .plus
	mov rdi,'-'
	push rax
	call print_char
	pop rax
	neg rax
	.plus:
	mov rdi, rax
	call print_uint
    ret

global string_equals:
string_equals:
    xor rax, rax
    .loop:
		mov al, byte[rdi]
		cmp al, byte[rsi]
		jne .false
		inc rdi
		inc rsi
		test al, al
		jne .loop
	mov rax, 1
	ret   
    .false:
    xor rax, rax
    ret

section .data
in_fd: dq 0

section .text
global read_char:
read_char:
    push 0
    xor rax, rax
    mov rdi, [in_fd]
    mov rsi, rsp 
    mov rdx, 1
    syscall
    pop rax
    ret 
    
section .data
word_buffer times 256 db 0

; rdi - buffer
; rsi - count
section .text
global read_word:
read_word:
	cmp rsi, 1
	jb .end

    push r14
    xor r14, r14  
	
    .A:
    push rdi
    call read_char
    pop rdi
    cmp al, ' '
    je .A
    cmp al, 10
    je .A
    cmp al, 13
    je .A 
    cmp al, 9 
    je .A
    test al, al
    jz .C

    .B:
    mov byte [rdi + r14], al
    inc r14

    push rdi
    call read_char
    pop rdi
    cmp al, ' '
    je .C
    cmp al, 10
    je .C
    cmp al, 13
    je .C 
    cmp al, 9
    je .C
    test al, al
    jz .C
    cmp r14, 254
    je .C 

    jmp .B

    .C:
    mov byte [rdi + r14], 0
    mov rax, rdi 
   
    mov rdx, r14 
    pop r14
.end:
    ret
   
global read_string
read_string:
	xor r8, r8
	mov rsi, word_buffer
	mov rdx, 1 
	mov rdi, 0
	mov byte[word_buffer], 0x00
	.loop:
		xor rax, rax
		lea rsi, [word_buffer + r8]
		syscall
		test al ,al
		je .end
		cmp byte[word_buffer + r8], 0xA
		je .end
		cmp r8, 255
		je .end
		inc r8
		jmp .loop
	.end:
	mov byte[word_buffer + r8], 0
	mov rax, word_buffer
	mov rdx, r8
	ret

; rdi points to a string
; returns rax: number, rdx : length
global parse_uint:
parse_uint:
    xor rax, rax
    xor rdx, rdx
    xor r8, r8
    mov r9, rdi
    .loop1:
		cmp byte[r9], '0'
		jb .nloop
		cmp byte[r9], '9'
		ja .nloop
		inc r9
		inc rdx
		jmp .loop1
	.nloop:
	push rdx
	cmp r9, rdi
	je .end
	mov r8, 1
	xor r10, r10
	mov r11, 10
	.loop2:
		dec r9
		xor rax, rax
		mov al, byte[r9]
		sub al, '0'
		mul r8
		add r10, rax
		mov rax, r8
		mul r11
		mov r8, rax
		
		cmp r9, rdi
		jbe .end
		jmp .loop2
		
	.end:
	pop rdx
	mov rax, r10
    ret


; rdi points to a string
; returns rax: number, rdx : length
global parse_int:
	xor rdx, rdx
	xor rax, rax
	cmp byte[rdi], '-'
	jne .plus
	inc rdi
	call parse_uint
	inc rdx
	cmp rdx, 0
	jnz .cont
	ret	
	.cont:
	neg rax
	ret
	.plus:
	call parse_uint
	ret 


global string_copy:
string_copy:
	xor rax, rax
	.loop:
		mov cl, [rdi]
		mov [rsi], cl
		inc rdi
		inc rsi
		cmp cl, 0
		jnz .loop
    ret
