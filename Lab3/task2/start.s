section .data
strlen_msg db "Hello, Infected File", 0x0A

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
    ; Function argument: filename is passed in ebx

    ; Print the filename
    push   ebx               ; push filename pointer
    call   print_filename    ; call helper function to print filename
    add    esp, 4            ; clean up stack

    ; Open the file for appending
    mov    eax, 5            ; SYS_OPEN system call
    mov    ecx, 0x202        ; O_WRONLY | O_APPEND | O_CREAT
    int    0x80              ; invoke system call
    mov    edi, eax          ; save file descriptor to edi

    ; Check for error (eax will be negative if there's an error)
    cmp    eax, 0xFFFFFFFF
    je     open_failed       ; jump if error

    ; Write infection message to file
    mov    eax, edi          ; file descriptor
    mov    ebx, infection_msg ; buffer address
    mov    ecx, strlen(strlen_msg) ; length of infection message
    mov    edx, 0            ; no additional flags
    mov    esi, 4            ; SYS_WRITE system call
    int    0x80              ; invoke system call

    ; Close the file
    mov    eax, 6            ; SYS_CLOSE system call
    mov    ebx, edi          ; file descriptor
    int    0x80              ; invoke system call

    ; Return to caller
    ret

open_failed:
    ; Handle error case (print error message, etc.)
    ; You can implement error handling as per your requirement
    ret

print_filename:
    ; Helper function to print the filename
    pop    ecx               ; filename pointer
    mov    edx, ecx          ; length of the filename
    mov    ebx, 1            ; file descriptor for STDOUT
    mov    eax, 4            ; SYS_WRITE system call
    int    0x80              ; invoke system call
    ret

    
