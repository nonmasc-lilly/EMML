INCLUDE "bool.mc"

LABEL _start START
    WHILE 1 DO
        ASM "push '0'"
        ASM "mov rax, 1"
        ASM "mov rdi, 1"
        ASM "mov rsi, rsp"
        ASM "mov rdx, 1"
        ASM "syscall"
        ASM "pop rax"
    STOP
END
