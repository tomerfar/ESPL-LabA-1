section .data
strlen_msg db "Hello, Infected File", 0x0A
newline db 10 ; newline character

section .text
global system_call
global infection
global infector 
extern main
extern strlen


_start:
    pop    ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    ;; lea eax, [esi+4*ecx+4] ; eax = envp = (4*ecx)+esi+4
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv 
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc

    call    main        ; int main( int argc, char *argv[], char *envp[] )

    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop
       
system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...        
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

    infection:
    ; Print "Hello, Infected File" using system_call
    mov    edx, 21            ; length of the message
    mov    ecx, strlen_msg ; address of the message
    mov    ebx, 1             ; file descriptor for STDOUT
    mov    eax, 4             ; SYS_WRITE
    int    0x80               ; invoke system call
    ret

    infector:
    ; Save registers that will be used
    push   ebp
    mov    ebp, esp
    sub    esp, 4

    ; Print the filename
    mov    ecx, [ebp+8]    ; filename (argument to infector)
    push   ecx
    call   strlen
    add    esp, 4
    mov    edx,eax
    mov    eax, 4
    mov    ebx, 1          ; file descriptor: stdout
    mov    ecx, [ebp+8]
    int    0x80

    ; Write newline
    mov eax, 4
    push ebx              ; save ebx (argc)
    mov ebx, 1            ; file descriptor 1 is stdout
    lea ecx, [newline]    ; address of newline character
    mov edx, 1            ; length of newline
    int 0x80
    pop ebx               ; restore ebx (argc)

    ; Open the file (append mode)
    mov    eax, 5          ; syscall: open
    mov    ebx, [ebp+8]    ; filename
    mov    ecx, 0x401      ; flags: O_APPEND | O_WRONLY
    mov    edx, 0644       ; mode: 0644
    int    0x80
    test   eax, eax        ; check if file descriptor is valid
    js     open_error      ; jump to open_error if error

    mov    ebx, eax        ; save file descriptor

    ; Seek to the end of the file
    mov eax, 19       ; SYS_LSEEK
    mov ecx, 0        ; Offset
    mov edx, 2        ; SEEK_END
    int 0x80

    ; Write the virus code to the file
    mov    eax, 4          ; syscall: write
    mov    ecx, code_start ; message to write (code_start)
    mov    edx, code_end - code_start  ; message length
    int    0x80

    ; Close the file
    mov    eax, 6          ; syscall: close
    int    0x80

    jmp    done

open_error:
    ; Handle open error
    mov    eax, 1          ; syscall: exit
    mov    ebx, 0x55       ; exit code
    int    0x80

done:
    ; Restore registers
    mov    esp, ebp
    pop    ebp
    ret

code_start:
    db "Hello, Infected File", 0x0A

code_end:
