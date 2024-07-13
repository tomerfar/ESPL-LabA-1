section .data
    hello db 'Hello, world!', 0

section .text
    global _start

_start:
    ; Write "Hello, world!" to stdout
    mov eax, 4          ; sys_write
    mov ebx, 1          ; file descriptor 1 is stdout
    mov ecx, hello      ; pointer to the hello message
    mov edx, 13         ; length of the hello message
    int 0x80            ; call kernel

    ; Exit the program
    mov eax, 1          ; sys_exit
    xor ebx, ebx        ; exit code 0
    int 0x80            ; call kernel
