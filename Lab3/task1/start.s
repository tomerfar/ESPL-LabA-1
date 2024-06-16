section .data
    newline db 10 ; newline character

section .bss
    buf resb 1024 ; buffer for command line arguments
    char resb 1 ; buffer for a single character
    input_fd resd 0 ; file descriptor for input
    output_fd resd 1 ; file descriptor for output

section .text
    extern strlen
    global _start

_start:
    ; Set up argc and argv
    mov ebx, [esp]         ; ebx = argc
    lea esi, [esp + 4]     ; esi = address of argv (array of pointers)
    dec ebx                ; decrement argc to exclude the program name
    add esi, 4             ; move esi to point to the first argument (argv[1])

print_args:
    cmp ebx, 0
    je encode ; jump to encode instead of exit_program

    ; Get the address of the current argument
    mov ecx, [esi]

    ; Check if the argument is an option to change input/output
    call Change_IO

    ; Save ECX before calling strlen
    push ecx

    ; Call strlen
    call strlen
    add esp, 4            ; clean up the stack after call to strlen

    ; Write the argument to stdout
    mov edx, eax          ; length of the string
    mov eax, 4            ; syscall number for sys_write
    push ebx              ; save ebx (argc)
    mov ebx, 1            ; file descriptor 1 is stdout
    mov ecx, [esi]        ; pointer to the string
    int 0x80
    pop ebx               ; restore ebx (argc)

    ; Write newline
    mov eax, 4
    push ebx              ; save ebx (argc)
    mov ebx, 1            ; file descriptor 1 is stdout
    lea ecx, [newline]    ; address of newline character
    mov edx, 1            ; length of newline
    int 0x80
    pop ebx               ; restore ebx (argc)

    ; Move to the next argument
    add esi, 4            ; move to the next argument
    dec ebx               ; decrement argument counter
    jmp print_args

Change_IO:
   cmp word [ecx], '-'+(256*'i')
   jz change_input
   cmp word [ecx], '-'+(256*'o')
   jz change_output
   ret

change_input:
    push ecx
    add ecx, 2
    mov eax, 5
    mov ebx, ecx
    xor ecx, ecx
    int 0x80
    mov [input_fd], eax
    pop ecx
    ret

change_output:
    push ecx
    add ecx, 2
    mov eax, 8
    mov ebx, ecx
    mov ecx, 0777
    int 0x80
    mov [output_fd], eax
    pop ecx
    ret
    
encode:
    ; Read a character from stdin
    mov eax, 3
    mov ebx, [input_fd]
    lea ecx, [char]
    mov edx, 1
    int 0x80

    ; Check if there are no more characters to read
    cmp eax, 0
    je exit_program

    ; Check if the character is 'z' or 'Z'
    cmp byte [char], 'z'
    je wrap_z
    cmp byte [char], 'Z'
    je wrap_Z

    ; Check if the character is in the range 'A' to 'y' or 'a' to 'y'
    cmp byte [char], 'A'
    jl write_char
    cmp byte [char], 'y'
    jg check_lower
    jmp encode_char

check_lower:
    cmp byte [char], 'a'
    jl write_char
    cmp byte [char], 'y'
    jg write_char

encode_char:
    ; Encode the character by adding 1 to its value
    inc byte [char]
    jmp write_char

wrap_z:
    ; Wrap 'z' to 'a'
    sub byte [char], 25
    jmp write_char

wrap_Z:
    ; Wrap 'Z' to 'A'
    sub byte [char], 25

write_char:
    ; Write the encoded character to stdout
    mov eax, 4
    mov ebx, [output_fd]
    lea ecx, [char]
    mov edx, 1
    int 0x80

    ; Repeat the process
    jmp encode

exit_program:
    ; Exit the program
    mov eax, 1            ; syscall number for sys_exit
    xor ebx, ebx          ; exit status 0
    int 0x80