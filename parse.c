#include "main.h"

struct AST *ast_new() {
    struct AST *ret;
    ret = malloc(AST_SIZE);
    ret->children = malloc(1);
    ret->child_num = 0;
    return ret;
}

static struct AST *parse_int(struct token **lexed, int *offset, int lsz) {
    struct AST *ret;
    if(*offset > lsz-1) return NULL;
    if(lexed[*offset]->id != t_int) return NULL;
    ret = AST_NEW();
    ret->id = t_int;
    ret->value = STR_DUP(lexed[*offset]->value);
    (*offset)++;
    return ret;
}

static struct AST *parse_iden(struct token **lexed, int *offset, int lsz) {
    struct AST *ret;
    if(*offset > lsz-1) return NULL;
    if(lexed[*offset]->id != t_iden) return NULL;
    ret = AST_NEW();
    ret->id = t_iden;
    ret->value = STR_DUP(lexed[*offset]->value);
    (*offset)++;
    return ret;
}

static struct AST *parse_exit(struct token **lexed, int *offset, int lsz) {
    struct AST *ret, *_int;
    if(*offset > lsz-1) return NULL;
    if(lexed[*offset]->id != t_exit) return NULL;
    (*offset)++;
    _int = parse_int(lexed, offset, lsz);
    if(_int == NULL) return NULL;
    ret = AST_NEW();
    ret->id = t_exit;
    ret->value = NULL;
    AST_CHILD_ADD(ret, _int);
    return ret;
}

static struct AST *parse_label(struct token **lexed, int *offset, int lsz) {
    struct AST *ret, *_iden;
    if(*offset > lsz-1) return NULL;
    if(lexed[*offset]->id != t_label) return NULL;
    (*offset)++;
    _iden = parse_iden(lexed, offset, lsz);
    if(_iden == NULL) return NULL;
    ret = AST_NEW();
    ret->id = t_label;
    ret->value = NULL;
    AST_CHILD_ADD(ret, _iden);
    return ret;
}

static struct AST *parse_statement(struct token **lexed, int *offset, int lsz) {
    struct AST *statements[] = { parse_exit(lexed, offset, lsz),
        parse_label(lexed, offset, lsz) };
    int i;
    for(i=0; i < sizeof(statements); i++)
        if(statements[i] != NULL) return statements[i];
    return NULL;
}

struct AST *parse(struct token **lexed, int lsz) {
    struct AST *ret, *tmp;
    int offset;

    ret = AST_NEW();
    ret->id = t_root;
    ret->value = NULL;
    offset = 0;
    while(1) {
        if(tmp = parse_statement(lexed, &offset, lsz)) {
            repast(tmp, 0);
            AST_CHILD_ADD(ret, tmp);
        }
        else break;
        if(offset > lsz-1) break;
    }
    return ret;
}

int repast(struct AST *a, int n) {
    int i;
    for(i=0; i < n; i++) printf("|");
    printf("> {%s : %s}\n", id_type(a->id),
        a->value == NULL ? "NULL" : a->value);
    for(i=0; i < a->child_num; i++)
        repast(a->children[i], n+1);
    return 0;
}


