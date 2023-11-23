#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#ifndef MAIN_H
#define MAIN_H

#define TOKEN_SIZE sizeof(struct token)
#define TOKEN_NEW() malloc(TOKEN_SIZE)
#define TOKEN_DEL(tok) if(tok->value) free(tok->value); free(tok)
#define LEX_NEW(input_string) lex(s)
#define LEX_DEL(_lex, sz) { int ___; for(___ = 0; ___ < sz; ___++) {\
        TOKEN_DEL(_lex[___]);\
    } } free(_lex)\

typedef enum token_types {
    t_root, t_exit, t_label, t_iden, t_int, t_hex,
} TOKEN_TYPE;

struct token {
    TOKEN_TYPE id;
    char *value;
};

char *STR_DUP(char *a);

const char *id_type(int t_id);
int reptok(struct token t, int i);
struct token **lex(char *s, int *lsz);

#define AST_SIZE sizeof(struct AST)
#define AST_NEW() ast_new();
#define AST_DEL(ast) {int ___; for(___ = 0; ___ < ast.child_num; ___++) { \
        AST_DEL(ast->children[___]);\
    } } free(ast->children); free(ast)
#define AST_CHILD_ADD(ast, child) do {\
        ast->children = realloc(ast->children, (++ast->child_num)\
            * AST_SIZE);\
        ast->children[ast->child_num-1]=child;\
    } while(0)

struct AST {
    TOKEN_TYPE id;
    char *value;
    struct AST **children;
    int child_num;
};

struct AST *ast_new();

int repast(struct AST *a, int n);
struct AST *parse(struct token **lexed, int lsz);


char *compile(struct AST *ast);


#endif
