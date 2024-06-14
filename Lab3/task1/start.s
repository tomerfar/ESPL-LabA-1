SECTION .data
    Infile  dd 0  ; file descriptor for input file
    Outfile dd 1  ; file descriptor for output file
    buffer  db 0  ; buffer to read input
    newline db 10 ; newline character

SECTION .text
; make main_function available externally
global main

main:    ; int main (int argc, char* argv[])
    push ebp
    mov ebp,esp

    ; Parse command line arguments
    mov ecx, [ebp+8] ; pointer to argv
    mov edx, [ebp+12] ; argc

parse_args:
    dec edx
    js end_parse_args
    mov eax, [ecx+edx*4] ; argv[i]
    cmp byte [eax], '-'
    jne parse_args
    inc eax
    cmp byte [eax], 'i'
    je open_input
    cmp byte [eax], 'o'
    je open_output
    jmp parse_args

open_input:
    inc eax
    mov ebx, eax ; pointer to file name
    mov eax, 5 ; syscall number for sys_open
    xor ecx, ecx ; flags
    xor edx, edx ; mode
    int 0x80
    test eax, eax
    js end_parse_args ; if open failed, skip to end
    mov [Infile], eax
    jmp parse_args

open_output:
    inc eax
    mov ebx, eax ; pointer to file name
    mov eax, 5 ; syscall number for sys_open
    mov ecx, 1 ; flags (O_WRONLY)
    xor edx, edx ; mode
    int 0x80
    test eax, eax
    js end_parse_args ; if open failed, skip to end
    mov [Outfile], eax
    jmp parse_args

end_parse_args:

    ; Read from input file, encode, and write to output file
    mov eax, 3 ; syscall number for sys_read
    mov ebx, [Infile] ; file descriptor
    lea ecx, [buffer] ; buffer
    mov edx, 1 ; count
    int 0x80
    cmp eax, 0 ; check if EOF
    je end

    ; Encode only if character is in range 'A' to 'z'
    cmp byte [buffer], 'A'
    jl write_output
    cmp byte [buffer], 'Z'
    jg check_lower
    add byte [buffer], 1
    cmp byte [buffer], 'Z'+1
    jne write_output
    mov byte [buffer], 'A'
    jmp write_output

check_lower:
    cmp byte [buffer], 'a'
    jl write_output
    cmp byte [buffer], 'z'
    jg write_output
    add byte [buffer], 1
    cmp byte [buffer], 'z'+1
    jne write_output
    mov byte [buffer], 'a'

write_output:
    ; Write to output file
    mov eax, 4 ; syscall number for sys_write
    mov ebx, [Outfile] ; file descriptor
    lea ecx, [buffer] ; buffer
    mov edx, 1 ; count
    int 0x80
    jmp end_parse_args

end:
    ; Close files
    mov eax, 6 ; syscall number for sys_close
    mov ebx, [Infile]
    int 0x80
    mov ebx, [Outfile]
    int 0x80

    ; Exit
    mov eax, 1 ; syscall number for sys_exit
    xor ebx, ebx ; exit code
    int 0x80