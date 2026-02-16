section .text
    global _start

_start:
    mov rax, 1          ; sys_write
    mov rdi, 1          ; stdout
    mov rsi, msg        ; address
    mov rdx, len        ; length
    syscall

    mov rax, 60         ; sys_exit
    xor rdi, rdi
    syscall

section .data
msg db "hello", 0xa
len equ $ - msg

