all: main.c main.h lex.c
	gcc main.c lex.c preprocessor.c parse.c scopes.c codegenL64.c -o mc -std=c89

debug: all test.mc
	./mc test.mc -o test.asm -d

test: all test.mc
	./mc test.mc -o test.asm
	nasm -felf64 test.asm -o test.o
	ld test.o -o test
