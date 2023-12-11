#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#ifndef MAIN_H
#define MAIN_H

#define IGNORE (void*)-1


#define TOKEN_SIZE sizeof(struct token)
#define TOKEN_NEW() malloc(TOKEN_SIZE)
#define TOKEN_DEL(tok) if(tok->value) free(tok->value); free(tok)
#define TOKEN_FROM(token, ret) do {\
        (ret) = TOKEN_NEW(); \
        (ret)->id = token->id; \
        if(token->value != NULL) (ret)->value = STR_DUP(token->value);\
    } while(0)
#define LEX_NEW(input_string) lex(s)
#define LEX_DEL(_lex, sz) { int ___; for(___ = 0; ___ < sz; ___++) {\
        TOKEN_DEL(_lex[___]);\
    } } free(_lex)\

#define ASSERT(b, str, where, line, file) if(!(b)) (printf(str), printf("\tat:\n\ttoken: %d\n"\
        "\tline: %d\n\tfile: %s\n", (where), (line), (file)),\
        exit(-1))

typedef enum token_types {
    t_root,         t_exit,     t_label,
    t_iden,         t_int,      t_hex,
    t_jump,         t_register, t_equ,
    t_if,           t_macro,    t_mend,
    t_include,      t_str_lit,  t_start,
    t_end,          t_declare,  t_char_type,
    t_short_type,   t_int_type, t_long_type,
    t_pointer_type, t_set,      t_char,
    t_asm,          t_add,      t_sub,
    t_mul,          t_div,
} TOKEN_TYPE;

struct token {
    TOKEN_TYPE id;
    int line;
    char *value;
    const char *file;
};

char *STR_DUP(char *a);

const char *id_type(int t_id);
int reptok(struct token t, int i);
struct token **lex(const char *file, char *s, int *lsz);

struct token_link {
    const char *id;
    struct token **tokens;
    int tsz;
};
struct token **preprocess(struct token **lexed, int lsz, int *rsz,
    struct token_link *link_prev, int lksz);


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
struct AST *parse(struct token **processed, int lsz);

#define SCOPE scope_node
#define SCOPE_SIZE sizeof(struct scope_node)
#define SCOPE_NEW() scope_new()
#define SCOPE_ADD(_parent, _child) do { \
        _parent->child = _child; \
        _child->parent = _parent; \
    } while(0)
#define SCOPE_DEL(scope) scope_del(scope)
#define VARIABLE_SIZE sizeof(struct variable)

int literal_types(int literal);
const char *word_from_size(int size);
const char *reg_from_size(int size);
int size_from_type(int type);

struct variable {
    int size;
    int type;
    int offset;
    const char *name;
    const char *file;
};

struct scope_node {
    struct scope_node *parent;
    int variable_length;
    int stack_size;
    struct variable *variables;
    struct scope_node *child;
};


struct scope_node *scope_new();
struct scope_node *scope_from_ast(struct AST *a, struct scope_node *parent);
void scope_del(struct scope_node *scope);
char *compile(struct AST *ast);


#endif
