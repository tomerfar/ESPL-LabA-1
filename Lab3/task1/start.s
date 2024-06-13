section .data
SYS_WRITE equ 4
STDOUT equ 1
SYS_EXIT equ 1
newline db 10

section .text
extern strlen
global main

main:
    ; Save registers
    pusha

    ; Get argc and argv
    mov esi, [esp + 36] ; argc
    mov ecx, [esp + 40] ; argv

    ; Loop over arguments
    xor edi, edi ; Initialize loop counter to 0
.loop:
    ; Check if we've printed all arguments
    cmp edi, esi
    jge .end

    ; Get the address of the current argument
    mov ebx, [ecx + edi*4]

    ; Get the length of the argument
    push ebx
    call strlen
    add esp, 4
    mov edx, eax ; Store the length in edx

    ; Print the argument
    mov ecx, ebx ; Store the pointer to the argument in ecx
    mov eax, SYS_WRITE
    mov ebx, STDOUT
    int 0x80

    ; Print a newline
    mov ecx, newline ; Store the pointer to the newline character in ecx
    mov eax, SYS_WRITE
    mov ebx, STDOUT
    mov edx, 1
    int 0x80

    ; Move to the next argument
    inc edi
    add ecx, 4 ; Move to the next argument
    jmp .loop

.end:
    ; Restore registers
    popa

    ; Exit
    mov eax, SYS_EXIT
    xor ebx, ebx
    int 0x80