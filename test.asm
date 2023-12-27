section .text
  global _start

; label ;
_start:
  ; argset ;
  mov rax, 0
  mov rdi, rax
  mov rax, 1
  mov rsi, rax
  mov rax, 2
  mov rdx, rax
  ; syscall exit ;
  mov rax, rsi

  add rsp, 0
  mov rdi, rax
  mov rax, 60
  syscall

