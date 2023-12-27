section .text
  global _start

; label ;
_start:
  ; while ;
  .w0:  mov rax, 1

  or rax, rax
  jz .w0o
push '0'
mov rax, 1
mov rdi, 1
mov rsi, rsp
mov rdx, 1
syscall
pop rax

  jmp .w0
  .w0o:

