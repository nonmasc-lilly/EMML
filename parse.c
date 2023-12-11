#include "main.h"

#define BOUNDS_ASSERT(at) ASSERT(offset < lsz, "ERROR statement incomplete before EOF\n",\
        at, -1, "UNK")

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

static struct AST *parse_char(struct token **lexed, int offset, int lsz, int *outoff) {
    struct AST *ret;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_char) return NULL;
    ret = AST_NEW();
    ret->id = t_char;
    ret->value = STR_DUP(lexed[offset]->value);
    *outoff += 1;
    return ret;
}

static struct AST *parse_str(struct token **lexed, int offset, int lsz, int *outoff) {
    struct AST *ret;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_str_lit) return NULL;
    ret = AST_NEW();
    ret->id = t_str_lit;
    ret->value = STR_DUP(lexed[offset]->value);
    *outoff += 1;
    return ret;
}

static struct AST *parse_type(struct token **lexed, int offset, int lsz, int *outoff) {
    struct AST *ret;
    BOUNDS_ASSERT(offset);
    switch(lexed[offset]->id) {
    case t_char_type:
    case t_short_type:
    case t_int_type:
    case t_long_type:
    case t_pointer_type:
        ret = AST_NEW();
        ret->id = lexed[offset]->id;
        break;
    default: ret = NULL;
    }
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
    ASSERT(_expr1 != NULL, "expected 2 expressions after '='\n", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    _expr2 = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff);
    ASSERT(_expr2 != NULL, "expected 2 expressions after '='\n", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
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
    ASSERT(_expr != NULL, "expected int const after exit\n", offset + inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    ret = AST_NEW();
    ret->id = t_exit;
    ret->value = NULL;
    AST_CHILD_ADD(ret, _expr);
    *outoff += inoutoff;
    return ret;
}

static struct AST *parse_asm(struct token **lexed, int offset, int lsz, int *outoff) {
    struct AST *ret, *_str;
    int inoutoff;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_asm) return NULL;
    inoutoff=1;
    _str = parse_str(lexed, offset+inoutoff, lsz, &inoutoff);
    ASSERT(_str != NULL, "expected string literal after asm directive\n", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    ret = AST_NEW();
    ret->id = t_asm;
    ret->value = NULL;
    AST_CHILD_ADD(ret, _str);
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
    ASSERT(_iden != NULL, "ERROR expected iden after label\n", offset + inoutoff,
        lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
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
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_register) return NULL;
    inoutoff = 1;
    _iden = parse_iden(lexed, offset+inoutoff, lsz, &inoutoff);
    ASSERT(_iden != NULL, "ERROR expected register after register statement\n",
        offset + inoutoff, lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    ASSERT(IS_REGISTER(_iden->value), "ERROR expected an register after register"
        "statement\n", offset + inoutoff, lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    _expr = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff);
    ASSERT(_expr != NULL, "ERROR expected integer after register\n", offset + inoutoff,
        lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
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
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_jump) return NULL;
    inoutoff=1;
    _iden = parse_iden(lexed, offset+inoutoff, lsz, &inoutoff);
    ASSERT(_iden != NULL, "ERROR expected iden after jump\n", offset + inoutoff,
        lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    ret = AST_NEW();
    ret->value = NULL;
    if(lexed[offset+inoutoff]->id == t_if) {
        inoutoff+=1;
        type = t_if;
        _expr = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff);
        ASSERT(_expr != NULL, "expected expression after jump if statement\n", 
            offset+inoutoff, lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
        AST_CHILD_ADD(ret, _expr);
    } else type = t_jump;
    ret->id = type;
    AST_CHILD_ADD(ret, _iden);
    *outoff += inoutoff;
    return ret;
}

static struct AST *parse_declaration(struct token **lexed, int offset, int lsz,
        int *outoff) {
    struct AST *ret, *_type, *_iden;
    int inoutoff;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_declare) return NULL;
    inoutoff = 1;
    _type = parse_type(lexed, offset+inoutoff, lsz, &inoutoff);
    ASSERT(_type != NULL, "expected type after declaration\n", offset + inoutoff,
        lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    _iden = parse_iden(lexed, offset+inoutoff, lsz, &inoutoff);
    ASSERT(_iden != NULL, "expected iden after declaration\n", offset + inoutoff,
        lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    ret = AST_NEW();
    ret->id = t_declare;
    ret->value = NULL;
    AST_CHILD_ADD(ret, _type);
    AST_CHILD_ADD(ret, _iden);
    *outoff += inoutoff;
    return ret;
}

static struct AST *parse_statement(struct token **lexed, int offset, int lsz,
        int *outoff, int isroot);

static struct AST *parse_scope(struct token **lexed, int offset, int lsz, int *outoff) {
    struct AST *ret, *tmp;
    int inoutoff, inc;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_start) return NULL;
    ret = AST_NEW();
    ret->id = t_start;
    ret->value = NULL;
    inoutoff=1;
    while(1) {
        ASSERT(offset+inoutoff < lsz, "expected end to scope\n", offset,
            lexed[offset]->line, lexed[offset]->file);
        if(lexed[offset+inoutoff]->id == t_end) break;
        inc = 0;
        if((tmp = parse_statement(lexed, offset+inoutoff, lsz, &inc, 0)) != NULL) {
            inoutoff += inc;
            AST_CHILD_ADD(ret, tmp);
        } else ASSERT(0, "expected statement\n", offset, lexed[inoutoff+offset]->line,
            lexed[offset+inoutoff]->file);
    }
    *outoff += inoutoff+1;
    return ret;
}

static struct AST *parse_set(struct token **lexed, int offset, int lsz, int *outoff) {
    struct AST *ret, *_iden, *_expr;
    int inoutoff;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_set) return NULL;
    inoutoff = 1;

    _iden = parse_iden(lexed, offset+inoutoff, lsz, &inoutoff);
    ASSERT(_iden != NULL, "expected identifier after set statement\n", offset+inoutoff,
        lexed[inoutoff+offset]->line, lexed[offset+inoutoff]->file);
    _expr = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff);
    ASSERT(_expr != NULL, "expected expression after set statement\n", offset+inoutoff,
        lexed[inoutoff+offset]->line, lexed[offset+inoutoff]->file);

    ret = AST_NEW();
    ret->id = t_set;
    AST_CHILD_ADD(ret, _iden);
    AST_CHILD_ADD(ret, _expr);
    *outoff += inoutoff;
    return ret;
}

#define STATEMENT_LEN 8
static struct AST *parse_statement(struct token **lexed, int offset, int lsz,
        int *outoff, int isroot) {
    struct AST *statements[STATEMENT_LEN] = { parse_exit(lexed, offset, lsz, outoff),
        parse_label(lexed, offset, lsz, outoff), parse_jump(lexed, offset, lsz, outoff),
        parse_register(lexed, offset, lsz, outoff),
        parse_scope(lexed, offset, lsz, outoff),
        parse_declaration(lexed, offset, lsz, outoff),
        parse_set(lexed, offset, lsz, outoff), parse_asm(lexed, offset, lsz, outoff) };
    int i;
    for(i=0; i < STATEMENT_LEN; i++)
        if(statements[i] != NULL) {
            ASSERT(!(!isroot && statements[i]->id == t_label),
                "labels not allowed in non root scopes\n", offset+*outoff,
                lexed[offset+*outoff]->line, lexed[offset+*outoff]->file);
            return statements[i];
        }
    return NULL;
}

struct AST *parse(struct token **lexed, int lsz) {
    struct AST *ret, *tmp;
    int offset, inc;

    ret = AST_NEW();
    ret->id = t_start;
    ret->value = NULL;
    offset = 0;
    while(1) {
        inc = 0;
        if((tmp = parse_statement(lexed, offset, lsz, &inc, 1)) != NULL) {
            offset += inc;
            AST_CHILD_ADD(ret, tmp);
        } else ASSERT(0, "expected statement\n", offset, lexed[offset-1]->line, lexed[offset-1]->file);
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


