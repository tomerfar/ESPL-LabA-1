section .data
    hello db 'Hello, World!', 0x0A  ; Our string
    helloLen equ $ - hello           ; The length of our string

section .text
    global _start                    ; The entry point for our program

_start:
    ; Write 'Hello, World!' to stdout
    mov eax, 4                       ; The syscall number for sys_write
    mov ebx, 1                       ; File descriptor 1 is stdout
    mov ecx, hello                   ; Pointer to the message
    mov edx, helloLen                ; Length of the message
    int 0x80                         ; Call the kernel

    ; Exit the program
    mov eax, 1                       ; The syscall number for sys_exit
    xor ebx, ebx                     ; Exit code 0
    int 0x80                         ; Call the kernel