# EMML

Educational Memory Management Language is a language designed to teach the concepts required for low level programming


Example program:
```
LABEL _start
EXIT 0
```

compiles to:
```x86asm
section .text
  global _start

_start:
  mov rax, 0
  mov rdi, rax
  mov rax, 60
  syscall
```

TODO:

\- = what i am planning to work on

~ = something i am currently working on

\# = something i have finished working on

X = something that i have found a bug to fix


- [#] add labels
- [#] add exit
- [-] add subroutines (functions without arguments passed through)
- [#] add jumps (goto in C for example, callsub for subroutines)
- [-] add conditional jumps
- [~] add register manipulation
- [-] add stack variables
- [-] add operations (add, sub, mul, etc)
- [-] add scopes
- [-] add syscall capability
- [-] add include statements (similar to C includes)
- [-] add procedures (this is function in the programming way)
- [-] add functions (this is function in the math sense (i.e lambda functions))
- [-] add macros
- [-] prove turing completeness
- [-] make self hosting
