all: main.c main.h lex.c
	gcc main.c lex.c parse.c codegenL64.c -o mc -std=c89

test: all test.mc
	./mc test.mc -o test.asm
	nasm -felf64 test.asm -o test.o
	ld test.o -o test
