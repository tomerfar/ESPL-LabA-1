; Save this as test.asm

section .text
    global _start

_start:
    ; Exit the program
    mov eax, 1          ; sys_exit
    xor ebx, ebx        ; exit code 0
    int 0x80            ; interrupt to make the system call

section .data
    ; Some data, not used in the program
    db 'Hello, World!', 0