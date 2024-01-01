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

enum { GET, SET, START, END,
       TYPE };
struct pvar {
    int type;
    char *id;
};
int variable_function(int action, const char *id, int *info, int ret_type) {
    int i;
    static int size = 0;
    static struct pvar *vars = NULL;
    switch(action) {
    case START:
        vars = malloc(1);
        break;
    case END:
        for(i = 0; i < size; i++)
            free(vars[i].id);
        free(vars);
        break;
    case GET:
        for(i = 0; i < size; i++)
            if(!strcmp(vars[i].id, id))
                switch(ret_type) {
                case TYPE: return vars[i].type;
                }
        return -1;
    case SET:
        vars = realloc(vars, sizeof(struct pvar)+(size+1));
        vars[size].id = STR_DUP(id);
        vars[size].type = info[0];
        size++;
        return 0;
    }
    return -2;
}

int in_subrt(int set, int type) {
    static int is_in_subrt = 0;
    static int ctype = 0;
    if(set == -1)
        return type ? ctype : is_in_subrt;
    is_in_subrt = set;
    type = type;
    return set;
}

int add_subrt(const char *name, int type, int get) {
    static const char **subrts = NULL;
    static int *types = NULL;
    static int len = 0;
    int i;
    if(subrts == NULL) subrts = malloc(1);
    if(types == NULL)  types = malloc(1);
    if(get) {
        for(i = 0; i < len; i++)
            if(!strcmp(name, subrts[i])) return type ? types[i] : 1;
        return 0;
    }
    subrts = realloc(subrts, (len+1)*sizeof(const char*));
    types  = realloc(types,  (len+1)*sizeof(int));
    subrts[len] = name;
    types[len]  = type;
    len++;
}

static struct AST *parse_expr(struct token **lexed, int offset, int lsz, int *outoff, int *type);

static struct AST *parse_int(struct token **lexed, int offset, int lsz, int *outoff, int *type) {
    struct AST *ret;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_int) return NULL;
    ret = AST_NEW();
    ret->id = t_int;
    ret->value = STR_DUP(lexed[offset]->value);
    *outoff += 1;
    if(type != NULL) *type = t_int_type;
    return ret;
}

static struct AST *parse_char(struct token **lexed, int offset, int lsz, int *outoff, int *type) {
    struct AST *ret;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_char) return NULL;
    ret = AST_NEW();
    ret->id = t_char;
    ret->value = STR_DUP(lexed[offset]->value);
    *outoff += 1;
    if(type != NULL) *type = t_char_type;
    return ret;
}

static struct AST *parse_str(struct token **lexed, int offset, int lsz, int *outoff, int *type) {
    struct AST *ret;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_str_lit) return NULL;
    ret = AST_NEW();
    ret->id = t_str_lit;
    ret->value = STR_DUP(lexed[offset]->value);
    *outoff += 1;
    if(type != NULL) *type = t_pointer_type;
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

static struct AST *parse_iden(struct token **lexed, int offset, int lsz, int *outoff, int *type) {
    struct AST *ret;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_iden) return NULL;
    ret = AST_NEW();
    ret->id = t_iden;
    ret->value = STR_DUP(lexed[offset]->value);
    *outoff += 1;
    if(type != NULL) *type = variable_function(GET, lexed[offset]->value, NULL, TYPE);
    return ret;
}

static struct AST *parse_equals(struct token **lexed, int offset, int lsz, int *outoff, int *type) {
    struct AST *ret, *_expr1, *_expr2;
    int inoutoff, t1, t2;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_equ) return NULL;
    inoutoff=1;
    _expr1 = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &t1);
    ASSERT(_expr1 != NULL, "expected 2 expressions after '='\n", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    _expr2 = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &t2);
    ASSERT(_expr2 != NULL, "expected 2 expressions after '='\n", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    ASSERT(int_type(t1) && int_type(t2), "expected integer expressions for equals\n", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    ret = AST_NEW();
    ret->id = t_equ;
    ret->value = NULL;
    AST_CHILD_ADD(ret, _expr1);
    AST_CHILD_ADD(ret, _expr2);
    *outoff += inoutoff;
    if(type != NULL) *type = t_int_type;
    return ret;
}

static struct AST *parse_add(struct token **lexed, int offset, int lsz, int *outoff, int *type) {
    struct AST *ret, *_expr1, *_expr2;
    int inoutoff, t1, t2;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_add) return NULL;
    inoutoff=1;
    _expr1 = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &t1);
    ASSERT(_expr1 != NULL, "expected 2 expressions after add expression\n", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    _expr2 = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &t2);
    ASSERT(_expr2 != NULL, "expected 2 expressions after add expression\b", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    ASSERT((int_type(t1) && int_type(t2)) || (t1 == t2 && t1 != t_alloc), "expected expressions"
        " of same type\n", offset+inoutoff, lexed[offset+inoutoff]->line,
        lexed[offset+inoutoff]->file);
    ret = AST_NEW();
    ret->id = t_add;
    ret->value = NULL;
    AST_CHILD_ADD(ret, _expr1);
    AST_CHILD_ADD(ret, _expr2);
    *outoff += inoutoff;
    if(type != NULL) *type = t1;
    return ret;
}

static struct AST *parse_sub(struct token **lexed, int offset, int lsz, int *outoff, int *type) {
    struct AST *ret, *_expr1, *_expr2;
    int inoutoff, t1, t2;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_sub) return NULL;
    inoutoff=1;
    _expr1 = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &t1);
    ASSERT(_expr1 != NULL, "expected 2 expressions after sub expression\n", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    _expr2 = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &t2);
    ASSERT(_expr2 != NULL, "expected 2 expressions after sub expression\n", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    ASSERT((int_type(t1) && int_type(t2)) || (t1 == t2 && t1 != t_alloc), "expected expressions"
        " of same type\n", offset+inoutoff, lexed[offset+inoutoff]->line,
        lexed[offset+inoutoff]->file);
    ret = AST_NEW();
    ret->id = t_sub;
    ret->value = NULL;
    AST_CHILD_ADD(ret, _expr1);
    AST_CHILD_ADD(ret, _expr2);
    *outoff += inoutoff;
    if(type != NULL) *type = t1;
    return ret;
}

static struct AST *parse_mul(struct token **lexed, int offset, int lsz, int *outoff, int *type) {
    struct AST *ret, *_expr1, *_expr2;
    int inoutoff, t1, t2;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_mul) return NULL;
    inoutoff=1;
    _expr1 = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &t1);
    ASSERT(_expr1 != NULL, "expected 2 expressions after mul expression\n", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    _expr2 = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &t2);
    ASSERT(_expr2 != NULL, "expected 2 expressions after mul expression\n", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    ASSERT(int_type(t1) && int_type(t2), "expected int or substitute expressions\n", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    ret = AST_NEW();
    ret->id = t_mul;
    ret->value = NULL;
    AST_CHILD_ADD(ret, _expr1);
    AST_CHILD_ADD(ret, _expr2);
    *outoff += inoutoff;
    if(type != NULL) *type = t_int_type;
    return ret;
}

static struct AST *parse_div(struct token **lexed, int offset, int lsz, int *outoff, int *type) {
    struct AST *ret, *_expr1, *_expr2;
    int inoutoff, t1, t2;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_div) return NULL;
    inoutoff=1;
    _expr1 = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &t1);
    ASSERT(_expr1 != NULL, "expected 2 expressions after div expression\n", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    _expr2 = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &t2);
    ASSERT(_expr2 != NULL, "expected 2 expressions after div expression\b", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    ASSERT(int_type(t1) && int_type(t2), "expected expressions to evaluate int or substitute\n",
        offset+inoutoff, lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    ret = AST_NEW();
    ret->id = t_div;
    ret->value = NULL;
    AST_CHILD_ADD(ret, _expr1);
    AST_CHILD_ADD(ret, _expr2);
    *outoff += inoutoff;
    if(type != NULL) *type = t_int_type;
    return ret;
}

static struct AST *parse_or(struct token **lexed, int offset, int lsz, int *outoff, int *type) {
    struct AST *ret, *_expr1, *_expr2;
    int inoutoff, t1, t2;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_or) return NULL;
    inoutoff=1;
    _expr1 = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &t1);
    ASSERT(_expr1 != NULL, "expected 2 expressions after or expression\n", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    _expr2 = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &t2);
    ASSERT(_expr2 != NULL, "expected 2 expressions after or expression\b", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    ASSERT(int_type(t1) && int_type(t2), "expected expressions to evaluate int or substitute\n",
        offset+inoutoff, lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    ret = AST_NEW();
    ret->id = t_or;
    ret->value = NULL;
    AST_CHILD_ADD(ret, _expr1);
    AST_CHILD_ADD(ret, _expr2);
    *outoff += inoutoff;
    if(type != NULL) *type = t_int_type;
    return ret;
}

static struct AST *parse_and(struct token **lexed, int offset, int lsz, int *outoff, int *type) {
    struct AST *ret, *_expr1, *_expr2;
    int inoutoff, t1, t2;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_and) return NULL;
    inoutoff=1;
    _expr1 = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &t1);
    ASSERT(_expr1 != NULL, "expected 2 expressions after and expression\n", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    _expr2 = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &t2);
    ASSERT(_expr2 != NULL, "expected 2 expressions after and expression\b", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    ASSERT(int_type(t1) && int_type(t2), "expected expressions to evaluate int or substitute\n",
        offset+inoutoff, lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    ret = AST_NEW();
    ret->id = t_and;
    ret->value = NULL;
    AST_CHILD_ADD(ret, _expr1);
    AST_CHILD_ADD(ret, _expr2);
    *outoff += inoutoff;
    if(type != NULL) *type = t_int_type;
    return ret;
}

static struct AST *parse_xor(struct token **lexed, int offset, int lsz, int *outoff, int *type) {
    struct AST *ret, *_expr1, *_expr2;
    int inoutoff, t1, t2;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_xor) return NULL;
    inoutoff=1;
    _expr1 = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &t1);
    ASSERT(_expr1 != NULL, "expected 2 expressions after xor expression\n", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    _expr2 = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &t2);
    ASSERT(_expr2 != NULL, "expected 2 expressions after xor expression\b", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    ASSERT(int_type(t1) && int_type(t2), "expected expressions to evaluate int or substitute\n",
        offset+inoutoff, lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    ret = AST_NEW();
    ret->id = t_xor;
    ret->value = NULL;
    AST_CHILD_ADD(ret, _expr1);
    AST_CHILD_ADD(ret, _expr2);
    *outoff += inoutoff;
    if(type != NULL) *type = t_int_type;
    return ret;
}

static struct AST *parse_not(struct token **lexed, int offset, int lsz, int *outoff, int *type) {
    struct AST *ret, *_expr1, *_expr2;
    int inoutoff, t1, t2;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_not) return NULL;
    inoutoff=1;
    _expr1 = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &t1);
    ASSERT(_expr1 != NULL, "expected expression after not expression\n", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    ASSERT(int_type(t1), "expected int type for not expression\n", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    ret = AST_NEW();
    ret->id = t_not;
    ret->value = NULL;
    AST_CHILD_ADD(ret, _expr1);
    *outoff += inoutoff;
    if(type != NULL) *type = t_int_type;
    return ret;
}

static struct AST *parse_ref(struct token **lexed, int offset, int lsz, int *outoff, int *type) {
    struct AST *ret, *_iden;
    int inoutoff;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_ref) return NULL;
    inoutoff=1;
    _iden = parse_iden(lexed, offset+inoutoff, lsz, &inoutoff, NULL);
    ASSERT(_iden != NULL, "expected identifier to reference\n", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    ret = AST_NEW();
    ret->id = t_ref;
    ret->value = NULL;
    AST_CHILD_ADD(ret, _iden);
    *outoff += inoutoff;
    if(type != NULL) *type = t_pointer_type;
    return ret;
}

static struct AST *parse_deref(struct token **lexed, int offset, int lsz, int *outoff, int *type) {
    struct AST *ret, *_type, *_expr;
    int inoutoff, t1;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_deref) return NULL;
    inoutoff=1;
    _type = parse_type(lexed, offset+inoutoff, lsz, &inoutoff);
    ASSERT(_type != NULL, "expected type for dereference\n", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    _expr = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &t1);
    ASSERT(_expr != NULL, "expected expression to dereference\n", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    ASSERT(t1 == t_pointer_type, "expected to dereference pointer\n", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    ret = AST_NEW();
    ret->id = t_deref;
    ret->value = NULL;
    AST_CHILD_ADD(ret, _expr);
    *outoff += inoutoff;
    if(type != NULL) *type = _type->id;
    return ret;
}

static struct AST *parse_get(struct token **lexed, int offset, int lsz, int *outoff, int *type) {
    struct AST *ret, *_iden, *_expr;
    int inoutoff, t1, t2;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_get) return NULL;
    inoutoff = 1;

    _expr = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &t1);
    ASSERT(_expr != NULL, "expected index for array get\n", offset+inoutoff,
        lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    ASSERT(int_type(t1), "expected index as int type\n", offset+inoutoff,
        lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    _iden = parse_iden(lexed, offset+inoutoff, lsz, &inoutoff, &t1);
    ASSERT(_iden != NULL, "expected array name for get\n", offset+inoutoff,
        lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    ret = AST_NEW();
    ret->id = t_get;
    AST_CHILD_ADD(ret, _iden);
    AST_CHILD_ADD(ret, _expr);
    *outoff += inoutoff;
    *type = t1;
    return ret;
}

static struct AST *parse_argget(struct token **lexed, int offset, int lsz, int *outoff, int *type) {
    struct AST *ret, *_int;
    int inoutoff, t1;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_argget) return NULL;
    inoutoff = 1;

    _int = parse_int(lexed, offset+inoutoff, lsz, &inoutoff, &t1);
    ASSERT(_int != NULL ,"expected constant int index of argget\n", offset+inoutoff,
        lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    ret = AST_NEW();
    ret->id = t_argget;
    ret->value = NULL;
    AST_CHILD_ADD(ret, _int);
    *outoff += inoutoff;
    *type = t_int_type;
    return ret;
}

static struct AST *parse_run(struct token **lexed, int offset, int lsz, int *outoff, int *type) {
    struct AST *ret, *_iden;
    int inoutoff;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_run) return NULL;
    inoutoff=1;
    _iden = parse_iden(lexed, offset+inoutoff, lsz, &inoutoff, NULL);
    ASSERT(_iden != NULL, "expected subrt to run\n", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    ASSERT(add_subrt(_iden->value, 0, 1), "expected run iden to be a subrt\n", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    ret = AST_NEW();
    ret->id = t_run;
    ret->value = NULL;
    AST_CHILD_ADD(ret, _iden);
    if(type != NULL) *type = add_subrt(_iden->value, 1, 1);
    *outoff += inoutoff;
    return ret;
}

#define EXPR_LEN 16
static struct AST *parse_expr(struct token **lexed, int offset, int lsz, int *outoff, int *type) {
    int i;
    struct AST *exprs[EXPR_LEN] = { parse_int(lexed, offset, lsz, outoff, type),
        parse_iden(lexed, offset, lsz, outoff, type),
        parse_equals(lexed, offset, lsz, outoff, type),
        parse_add(lexed, offset, lsz, outoff, type),
        parse_sub(lexed, offset, lsz, outoff, type),
        parse_mul(lexed, offset, lsz, outoff, type),
        parse_div(lexed, offset, lsz, outoff, type),
        parse_get(lexed, offset, lsz, outoff, type),
        parse_or(lexed, offset, lsz, outoff, type),
        parse_and(lexed, offset, lsz, outoff, type),
        parse_xor(lexed, offset, lsz, outoff, type),
        parse_not(lexed, offset, lsz, outoff, type),
        parse_ref(lexed, offset, lsz, outoff, type),
        parse_deref(lexed, offset, lsz, outoff, type),
        parse_argget(lexed, offset, lsz, outoff, type),
        parse_run(lexed, offset, lsz, outoff, type),
    };
    for(i = 0; i < EXPR_LEN; i++)
        if(exprs[i] != NULL) return exprs[i];
    return NULL;
}

static struct AST *parse_argset(struct token **lexed, int offset, int lsz, int *outoff) {
    struct AST *ret, *_expr[6] = {NULL};
    int inoutoff, i, j, t1;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_argset) return NULL;
    inoutoff=1;
    for(i = 0; i < 6; i++) {
        if(lexed[offset+inoutoff]->id == t_argend) break;
        _expr[i] = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &t1);
        ASSERT(_expr[i] != NULL, "expected 6 or less expressions for argset\n", inoutoff+offset,
            lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    }
    ASSERT(lexed[offset+inoutoff]->id == t_argend, "expected argend after 6 or less expressions of argset\n",
        inoutoff+offset, lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    inoutoff++;
    ret = AST_NEW();
    ret->id = t_argset;
    ret->value = NULL;
    for(j=0; j < i; j++)
        AST_CHILD_ADD(ret, _expr[j]);
    *outoff += inoutoff;
    return ret;
}

static struct AST *parse_exit(struct token **lexed, int offset, int lsz, int *outoff) {
    struct AST *ret, *_expr;
    int inoutoff, type;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_exit) return NULL;
    inoutoff=1;
    _expr = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &type);
    ASSERT(_expr != NULL, "expected expression after exit\n", offset + inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    ASSERT(int_type(type), "expected integer expression after exit\n", offset+inoutoff,
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
    _str = parse_str(lexed, offset+inoutoff, lsz, &inoutoff, NULL);
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
    _iden = parse_iden(lexed, offset+inoutoff, lsz, &inoutoff, NULL);
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
    int inoutoff, type;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_register) return NULL;
    inoutoff = 1;
    _iden = parse_iden(lexed, offset+inoutoff, lsz, &inoutoff, NULL);
    ASSERT(_iden != NULL, "ERROR expected register after register statement\n",
        offset + inoutoff, lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    ASSERT(IS_REGISTER(_iden->value), "ERROR expected an register after register"
        "statement\n", offset + inoutoff, lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    _expr = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &type);
    ASSERT(_expr != NULL, "ERROR expected expression after register\n", offset + inoutoff,
        lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    ASSERT(int_type(type), "expected integer expression\n", offset+inoutoff,
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
    int inoutoff, type, t1;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_jump) return NULL;
    inoutoff=1;
    _iden = parse_iden(lexed, offset+inoutoff, lsz, &inoutoff, NULL);
    ASSERT(_iden != NULL, "ERROR expected iden after jump\n", offset + inoutoff,
        lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    ret = AST_NEW();
    ret->value = NULL;
    if(lexed[offset+inoutoff]->id == t_if) {
        inoutoff+=1;
        type = t_if;
        _expr = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &t1);
        ASSERT(_expr != NULL, "expected expression after jump if statement\n", 
            offset+inoutoff, lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
        ASSERT(int_type(t1), "expected integer expression\n", offset+inoutoff,
            lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
        AST_CHILD_ADD(ret, _expr);
    } else type = t_jump;
    ret->id = type;
    AST_CHILD_ADD(ret, _iden);
    *outoff += inoutoff;
    return ret;
}

static struct AST *parse_alloc(struct token **lexed, int offset, int lsz, int *outoff) {
    struct AST *ret, *_int, *_iden, *_type;
    int inoutoff, info[1];
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_alloc) return NULL;
    inoutoff = 1;
    _int = parse_int(lexed, offset+inoutoff, lsz, &inoutoff, NULL);
    ASSERT(_int != NULL, "expected size for stack allocation\n", offset + inoutoff,
        lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    _type = parse_type(lexed, offset+inoutoff, lsz, &inoutoff);
    ASSERT(_type != NULL, "expected type for stack allocation\n", offset + inoutoff,
        lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    _iden = parse_iden(lexed, offset+inoutoff, lsz, &inoutoff, NULL);
    ASSERT(_iden != NULL, "expected iden for stack allocation\n", offset + inoutoff,
        lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    ret        = AST_NEW();
    ret->id    = t_alloc;
    ret->value = NULL;
    AST_CHILD_ADD(ret, _int);
    AST_CHILD_ADD(ret, _iden);
    AST_CHILD_ADD(ret, _type);
    info[0] = t_alloc;
    variable_function(SET, _iden->value, info, 0);
    *outoff += inoutoff;
    return ret;
}

static struct AST *parse_declaration(struct token **lexed, int offset, int lsz,
        int *outoff) {
    struct AST *ret, *_type, *_iden;
    int inoutoff, info[1];
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_declare) return NULL;
    inoutoff = 1;
    _type = parse_type(lexed, offset+inoutoff, lsz, &inoutoff);
    ASSERT(_type != NULL, "expected type after declaration\n", offset + inoutoff,
        lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    _iden = parse_iden(lexed, offset+inoutoff, lsz, &inoutoff, NULL);
    ASSERT(_iden != NULL, "expected iden after declaration\n", offset + inoutoff,
        lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    ret = AST_NEW();
    ret->id = t_declare;
    ret->value = NULL;
    AST_CHILD_ADD(ret, _type);
    AST_CHILD_ADD(ret, _iden);
    info[0] = _type->id;
    variable_function(SET, _iden->value, info, 0);
    *outoff += inoutoff;
    return ret;
}

static struct AST *parse_statement(struct token **lexed, int offset, int lsz,
        int *outoff, int isroot);

static struct AST *parse_while(struct token **lexed, int offset, int lsz, int *outoff) {
    struct AST *ret, *_expr, *tmp;
    int inoutoff, inc, t1;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_while) return NULL;
    inoutoff=1;
    _expr = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &t1);
    ASSERT(_expr != NULL, "expected expression for while loop\n", offset+inoutoff,
        lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    ASSERT(int_type(t1), "expected expression of while loop to be intish\n", offset+inoutoff,
        lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    ASSERT(lexed[offset+inoutoff]->id == t_do, "expected do statement to start while loop\n",
        offset+inoutoff, lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);
    inoutoff++;
    ret = AST_NEW();
    ret->id = t_while;
    ret->value = NULL;
    AST_CHILD_ADD(ret, _expr);
    while(1) {
        ASSERT(offset+inoutoff < lsz, "expected while to stop\n", offset,
            lexed[offset]->line, lexed[offset]->file);
        if(lexed[offset+inoutoff]->id == t_stop) break;
        inc = 0;
        if((tmp = parse_statement(lexed, offset+inoutoff, lsz, &inc, 0)) != NULL) {
            inoutoff += inc;
            AST_CHILD_ADD(ret, tmp);
        } else ASSERT(0, "expected statement\n", offset+inoutoff, lexed[inoutoff+offset]->line,
            lexed[inoutoff+offset]->file);
    }
    *outoff += inoutoff+1;
    return ret;
}

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
        } else ASSERT(0, "expected statement\n", offset+inoutoff, lexed[inoutoff+offset]->line,
            lexed[offset+inoutoff]->file);
    }
    *outoff += inoutoff+1;
    return ret;
}

static struct AST *parse_subrt(struct token **lexed, int offset, int lsz, int *outoff) {
    struct AST *ret, *_type, *_iden, *_scope;
    int inoutoff, prev, prevt;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_subrt) return NULL;
    inoutoff = 1;

    prev = in_subrt(-1, 0);
    prevt = in_subrt(-1, 1);

    _type = parse_type(lexed, offset+inoutoff, lsz, &inoutoff);
    ASSERT(_type != NULL, "expected type of subroutine\n", offset+inoutoff,
        lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    _iden = parse_iden(lexed, offset+inoutoff, lsz, &inoutoff, NULL);
    ASSERT(_iden != NULL, "expected name of subroutine\n", offset+inoutoff,
        lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    add_subrt(_iden->value, _type->id, 0);
    in_subrt(1, _type->id);
    _scope = parse_scope(lexed, offset+inoutoff, lsz, &inoutoff);
    ASSERT(_scope != NULL, "expected subroutine body scope\n", offset+inoutoff,
        lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    in_subrt(prev, prevt);
    ret = AST_NEW();
    ret->id = t_subrt;
    ret->value = NULL;
    AST_CHILD_ADD(ret, _type);
    AST_CHILD_ADD(ret, _iden);
    AST_CHILD_ADD(ret, _scope);
    *outoff += inoutoff;
    return ret;
}

static struct AST *parse_return(struct token **lexed, int offset, int lsz, int *outoff) {
    struct AST *ret, *_expr;
    int inoutoff, expected_type, t;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]-> id != t_return) return NULL;
    inoutoff=1;

    ASSERT(in_subrt(-1, 0), "return must be used in subrt\n", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->file);

    expected_type = in_subrt(-1, 1);
    _expr = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &t);
    ASSERT(_expr != NULL, "expected expression after return\n", offset+inoutoff,
        lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    ASSERT(t == expected_type || (int_type(t) && int_type(expected_type)), "expected return type to be the same as the subrts\n",
        offset+inoutoff, lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    ret = AST_NEW();
    ret->id = t_return;
    ret->value = NULL;
    AST_CHILD_ADD(ret, _expr);
    *outoff+=inoutoff;
    return ret;
}

static struct AST *parse_seta(struct token **lexed, int offset, int lsz, int *outoff) {
    struct AST *ret, *_iden, *_expr, *_expr2;
    int inoutoff, t1, t2;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_seta) return NULL;
    inoutoff = 1;

    _expr = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &t1);
    ASSERT(_expr2 != NULL, "expected index for array set\n", offset+inoutoff,
        lexed[inoutoff+offset]->line, lexed[offset+inoutoff]->file);
    ASSERT(int_type(t1), "expected int type for array index\n", offset+inoutoff,
        lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    _iden = parse_iden(lexed, offset+inoutoff, lsz, &inoutoff, &t1);
    ASSERT(_iden != NULL, "expected array identifier\n", offset+inoutoff,
        lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);
    _expr2 = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &t2);
    ASSERT(t1 == t2 || (int_type(t1) && int_type(t2)), "expected similar types for seta\n",
        offset+inoutoff, lexed[inoutoff+offset]->line, lexed[inoutoff+offset]->file);

    ret = AST_NEW();
    ret->id = t_seta;
    AST_CHILD_ADD(ret, _iden);
    AST_CHILD_ADD(ret, _expr);
    AST_CHILD_ADD(ret, _expr2);
    *outoff+=inoutoff;
    return ret;
}

static struct AST *parse_set(struct token **lexed, int offset, int lsz, int *outoff) {
    struct AST *ret, *_iden, *_expr, *_expr2;
    int inoutoff, t1, t2;
    BOUNDS_ASSERT(offset);
    if(lexed[offset]->id != t_set) return NULL;
    inoutoff = 1;
    
    _iden = parse_iden(lexed, offset+inoutoff, lsz, &inoutoff, &t1);
    ASSERT(_iden != NULL, "expected iden for set\n", offset+inoutoff,
        lexed[offset+inoutoff]->line, lexed[offset+inoutoff]->line);
    _expr = parse_expr(lexed, offset+inoutoff, lsz, &inoutoff, &t2);
    ASSERT(_expr != NULL, "expected expression after set statement\n", offset+inoutoff,
        lexed[inoutoff+offset]->line, lexed[offset+inoutoff]->file);
    ASSERT(t1 == t2 || (int_type(t1) && int_type(t2)), "expected similar types for set\n",
        offset+inoutoff, lexed[inoutoff+offset]->line, lexed[offset+inoutoff]->file);

    ret = AST_NEW();
    ret->id = t_set;
    AST_CHILD_ADD(ret, _iden);
    AST_CHILD_ADD(ret, _expr);
    *outoff += inoutoff;
    return ret;
}



#define STATEMENT_LEN 15
static struct AST *parse_statement(struct token **lexed, int offset, int lsz,
        int *outoff, int isroot) {
    struct AST *statements[STATEMENT_LEN] = { parse_exit(lexed, offset, lsz, outoff),
        parse_label(lexed, offset, lsz, outoff), parse_jump(lexed, offset, lsz, outoff),
        parse_register(lexed, offset, lsz, outoff),
        parse_scope(lexed, offset, lsz, outoff),
        parse_declaration(lexed, offset, lsz, outoff),
        parse_set(lexed, offset, lsz, outoff), parse_asm(lexed, offset, lsz, outoff),
        parse_alloc(lexed, offset, lsz, outoff), parse_seta(lexed, offset, lsz, outoff),
        parse_while(lexed, offset, lsz, outoff), parse_argset(lexed, offset, lsz, outoff),
        parse_subrt(lexed, offset, lsz, outoff), parse_return(lexed, offset, lsz, outoff),
        parse_run(lexed, offset, lsz, outoff, NULL), };
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

    variable_function(START, NULL, NULL, 0);

    while(1) {
        inc = 0;
        if((tmp = parse_statement(lexed, offset, lsz, &inc, 1)) != NULL) {
            offset += inc;
            AST_CHILD_ADD(ret, tmp);
        } else ASSERT(0, "expected statement\n", offset, lexed[offset-1]->line, lexed[offset-1]->file);
        if(offset > lsz-1) break;
    }
    variable_function(END, NULL, NULL, 0);
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


