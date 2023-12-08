#include "main.h"

#define BOUNDS_ASSERT(at) ASSERT(offset < lsz, "ERROR statement incomplete before EOF\n",\
        at)

#define IS_REGISTER(a) !strcmp(a, "A") || !strcmp(a, "B") || !strcmp(a, "X") \
        || !strcmp(a, "E")

struct AST *ast_new() {
    struct AST *ret;
    ret = malloc(AST_SIZE);
    ret->children = malloc(1);
    ret->child_num = 0;
    return ret;
}
static struct AST *parse_expr(struct token **lexed, int offset, int lsz, int *outoff);

static struct AST *parse_int(struct token **lexed, int offset, int lsz, int *outoff) {
    struct AST *ret;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_int) return NULL;
    ret = AST_NEW();
    ret->id = t_int;
    ret->value = STR_DUP(lexed[offset]->value);
    *outoff += 1;
    return ret;
}

static struct AST *parse_iden(struct token **lexed, int offset, int lsz, int *outoff) {
    struct AST *ret;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_iden) return NULL;
    ret = AST_NEW();
    ret->id = t_iden;
    ret->value = STR_DUP(lexed[offset]->value);
    *outoff += 1;
    return ret;
}

static struct AST *parse_equals(struct token **lexed, int offset, int lsz, int *outoff) {
    struct AST *ret, *_expr1, *_expr2;
    int inoutoff;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_equ) return NULL;
    inoutoff=1;
    _expr1 = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff);
    ASSERT(_expr1 != NULL, "expected 2 expressions after '='\n", offset+inoutoff);
    _expr2 = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff);
    ASSERT(_expr2 != NULL, "expected 2 expressions after '='\n", offset+inoutoff);
    ret = AST_NEW();
    ret->id = t_equ;
    ret->value = NULL;
    AST_CHILD_ADD(ret, _expr1);
    AST_CHILD_ADD(ret, _expr2);
    *outoff += inoutoff;
    return ret;
}

#define EXPR_LEN 3
static struct AST *parse_expr(struct token **lexed, int offset, int lsz, int *outoff) {
    int i;
    struct AST *exprs[EXPR_LEN] = { parse_int(lexed, offset, lsz, outoff),
        parse_iden(lexed, offset, lsz, outoff),
        parse_equals(lexed, offset, lsz, outoff) };
    for(i = 0; i < EXPR_LEN; i++)
        if(exprs[i] != NULL) return exprs[i];
    return NULL;
}

static struct AST *parse_exit(struct token **lexed, int offset, int lsz, int *outoff) {
    struct AST *ret, *_expr;
    int inoutoff;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_exit) return NULL;
    inoutoff=1;
    _expr = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff);
    ASSERT(_expr != NULL, "ERROR expected int const after exit\n", offset + inoutoff);
    ret = AST_NEW();
    ret->id = t_exit;
    ret->value = NULL;
    AST_CHILD_ADD(ret, _expr);
    *outoff += inoutoff;
    return ret;
}

static struct AST *parse_label(struct token **lexed, int offset, int lsz, int *outoff) {
    struct AST *ret, *_iden;
    int inoutoff;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_label) return NULL;
    inoutoff=1;
    _iden = parse_iden(lexed, offset+inoutoff, lsz, &inoutoff);
    ASSERT(_iden != NULL, "ERROR expected iden after label\n", offset + inoutoff);
    ret = AST_NEW();
    ret->id = t_label;
    ret->value = NULL;
    AST_CHILD_ADD(ret, _iden);
    *outoff += inoutoff;
    return ret;
}

static struct AST *parse_register(struct token **lexed, int offset, int lsz, int *outoff) {
    struct AST *ret, *_iden, *_expr;
    int inoutoff;
    if(offset > lsz-1) return NULL;
    if(lexed[offset]->id != t_register) return NULL;
    inoutoff = 1;
    _iden = parse_iden(lexed, offset+inoutoff, lsz, &inoutoff);
    ASSERT(_iden != NULL, "ERROR expected register after register statement\n",
        offset + inoutoff);
    ASSERT(IS_REGISTER(_iden->value), "ERROR expected an register after register"
        "statement\n", offset + inoutoff);
    _expr = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff);
    ASSERT(_expr != NULL, "ERROR expected integer after register\n", offset + inoutoff);
    ret = AST_NEW();
    ret->id = t_register;
    ret->value = NULL;
    AST_CHILD_ADD(ret, _iden);
    AST_CHILD_ADD(ret, _expr);
    *outoff += inoutoff;
    return ret;
}

static struct AST *parse_jump(struct token **lexed, int offset, int lsz, int *outoff) {
    struct AST *ret, *_iden, *_expr;
    int inoutoff, type;
    if(offset > lsz-1) return NULL;
    if(lexed[offset]->id != t_jump) return NULL;
    inoutoff=1;
    _iden = parse_iden(lexed, offset+inoutoff, lsz, &inoutoff);
    ASSERT(_iden != NULL, "ERROR expected iden after label\n", offset + inoutoff);
    ret = AST_NEW();
    ret->value = NULL;
    if(lexed[offset+inoutoff]->id == t_if) {
        inoutoff+=1;
        type = t_if;
        _expr = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff);
        ASSERT(_expr != NULL, "expected expression after jump if statement\n", 
            offset+inoutoff);
        AST_CHILD_ADD(ret, _expr);
    } else type = t_jump;
    ret->id = type;
    AST_CHILD_ADD(ret, _iden);
    *outoff += inoutoff;
    return ret;
}

#define STATEMENT_LEN 4
static struct AST *parse_statement(struct token **lexed, int offset, int lsz,
        int *outoff) {
    struct AST *statements[STATEMENT_LEN] = { parse_exit(lexed, offset, lsz, outoff),
        parse_label(lexed, offset, lsz, outoff), parse_jump(lexed, offset, lsz, outoff),
        parse_register(lexed, offset, lsz, outoff) };
    int i;
    for(i=0; i < STATEMENT_LEN; i++)
        if(statements[i] != NULL) return statements[i];
    return NULL;
}

struct AST *parse(struct token **lexed, int lsz) {
    struct AST *ret, *tmp;
    int offset, inc;

    ret = AST_NEW();
    ret->id = t_root;
    ret->value = NULL;
    offset = 0;
    while(1) {
        inc = 0;
        if((tmp = parse_statement(lexed, offset, lsz, &inc)) != NULL) {
            offset += inc;
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


