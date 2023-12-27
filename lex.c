#include "main.h"


#define IFNCMP(a, b) if(!strcmp(a, b))
#define ELNCMP(a, b) else IFNCMP(a, b)
static int tok(char *b) {
    int ret, isnum;
    char *si;
    IFNCMP(b, "")         ret = -1;
    ELNCMP(b, "START")    ret = t_start;
    ELNCMP(b, "END")      ret = t_end;
    ELNCMP(b, "EXIT")     ret = t_exit;
    ELNCMP(b, "LABEL")    ret = t_label;
    ELNCMP(b, "JUMP")     ret = t_jump;
    ELNCMP(b, "REGISTER") ret = t_register;
    ELNCMP(b, "IF")       ret = t_if;
    ELNCMP(b, "MACRO")    ret = t_macro;
    ELNCMP(b, "MEND")     ret = t_mend;
    ELNCMP(b, "INCLUDE")  ret = t_include;
    ELNCMP(b, "DECLARE")  ret = t_declare;
    ELNCMP(b, "CHAR")     ret = t_char_type;
    ELNCMP(b, "SHORT")    ret = t_short_type;
    ELNCMP(b, "INT")      ret = t_int_type;
    ELNCMP(b, "LONG")     ret = t_long_type;
    ELNCMP(b, "POINTER")  ret = t_pointer_type;
    ELNCMP(b, "SET")      ret = t_set;
    ELNCMP(b, "SETA")     ret = t_seta;
    ELNCMP(b, "ASM")      ret = t_asm;
    ELNCMP(b, "EQU")      ret = t_equ;
    ELNCMP(b, "ADD")      ret = t_add;
    ELNCMP(b, "SUB")      ret = t_sub;
    ELNCMP(b, "MUL")      ret = t_mul;
    ELNCMP(b, "DIV")      ret = t_div;
    ELNCMP(b, "OR")       ret = t_or;
    ELNCMP(b, "AND")      ret = t_and;
    ELNCMP(b, "XOR")      ret = t_xor;
    ELNCMP(b, "NOT")      ret = t_not;
    ELNCMP(b, "ALLOC")    ret = t_alloc;
    ELNCMP(b, "GETA")     ret = t_get;
    ELNCMP(b, "WHILE")    ret = t_while;
    ELNCMP(b, "DO")       ret = t_do;
    ELNCMP(b, "STOP")     ret = t_stop;
    ELNCMP(b, "REF")      ret = t_ref;
    ELNCMP(b, "DEREF")    ret = t_deref;
    ELNCMP(b, "ARGSET")   ret = t_argset;
    ELNCMP(b, "ARGEND")   ret = t_argend;
    ELNCMP(b, "ARGGET")   ret = t_argget;
    else {
        for(isnum=1,si=b; *si && isnum; si++) {
            isnum = isdigit(*si) || *si == '-';
        }
        ret = isnum ? t_int : t_iden;
    }
    return ret+1;
}
#undef IFNCMP
#undef ELNCMP

static int op(char c) {
    int ret;
    switch(c) {
    case '=': ret = t_equ;   break;
    case '+': ret = t_add;   break;
    case '-': ret = t_sub;   break;
    case '*': ret = t_mul;   break;
    case '/': ret = t_div;   break;
    case '{': ret = t_start; break;
    case '}': ret = t_end;   break;
    case '|': ret = t_or;    break;
    case '&': ret = t_and;   break;
    case '^': ret = t_xor;   break;
    case '~': ret = t_not;   break;
    default:  ret = -1;      break;
    }
    return ret+1;
}

static int tokhasarg(int tok) {
    int arg_toks[] = { t_int, t_iden },
        i = 0;
    for(i=0; i < sizeof(arg_toks); i++)
        if(tok == arg_toks[i]) return 1;
    return 0;
}

char *STR_DUP(const char *a) {
    char *ret;
    ret = malloc(strlen(a));
    strcpy(ret, a);
    return ret;
}

struct token **lex(const char *file, char *s, int *lsz) {
    char buff[1024] = {0}, *content, char_val[2] = {0};
    FILE *fp;
    int rsz, fsz, lsz2,
        comment, string,
        i, bp,
        line;
    char *si;
    struct token **ret, **append;

    line    = 0;
    string  = 0;
    comment = 0;
    ret = malloc(1);
    rsz = 0;
    bp=0;

    for(si=s; *si; si++) {
        if(*si == '.') continue;
        if(*si == '\n') line++;
        if(*si == '\"' && !comment) {
            if(string) {
                ret = realloc(ret, (rsz+1)*TOKEN_SIZE);
                ret[rsz]        = TOKEN_NEW();
                ret[rsz]->id    = t_str_lit;
                ret[rsz]->line  = line;
                ret[rsz]->file  = file;
                ret[rsz]->value = STR_DUP(buff);
                rsz++;
            }
            if(!string) {
                if(tok(buff)) {
                    ret = realloc(ret, (rsz+1)*TOKEN_SIZE);
                    ret[rsz] = TOKEN_NEW();
                    ret[rsz]->id = tok(buff)-1;
                    ret[rsz]->line = line;
                    ret[rsz]->file = file;
                    ret[rsz]->value = tokhasarg(tok(buff)-1) ? STR_DUP(buff) : NULL;
                    rsz++;
                }
            }
            buff[bp=0]=0;
            string = !string;
            continue;
        }
        if(*si == '\\' && !string) { comment = !comment; continue; }
        if(comment) continue;
        if(string) {
            buff[bp++] = *si;
            buff[bp]   = 0;
            continue;
        }
        if(*si == '\'' && *(si + 2) == '\'') {
            ret = realloc(ret, (rsz+1)*TOKEN_SIZE);
            ret[rsz] = TOKEN_NEW();
            ret[rsz]->id = t_char;
            ret[rsz]->line = line;
            ret[rsz]->file = file;
            char_val[0] = *(si + 1);
            ret[rsz]->value = STR_DUP(char_val);
            *si += 2;
            continue;
        }
        if(isspace(*si) || op(*si)) {
            if(buff[0] == 0) goto checkop;
            if(!tok(buff)) goto checkop;
            if(tok(buff)-1 == t_include) {
                i=0;
                while(*(si-1) != '\"') si++;
                buff[bp=0]=0;
                for(i=0; *si != '\"'; i++, si++) {
                    buff[bp++] = *si;
                    buff[bp] = 0;
                }
                fp = fopen(buff, "r");
                content = calloc(1,fsz=(fseek(fp, 0L, SEEK_END), ftell(fp)));
                rewind(fp);
                fread(content, 1, fsz, fp);
                fclose(fp);
                
                append = lex(buff, content, &lsz2);
                ret = realloc(ret, (rsz+lsz2)*TOKEN_SIZE);
                for(i = 0; i < lsz2; i++)
                    TOKEN_FROM(append[i], ret[i+rsz]);
                rsz+=lsz2;
                LEX_DEL(append, lsz2);
                
                free(content);
                buff[bp=0]=0;
                continue;
            }
            ret = realloc(ret, (rsz+1)*TOKEN_SIZE);
            ret[rsz] = TOKEN_NEW();
            ret[rsz]->id = tok(buff)-1;
            ret[rsz]->value = tokhasarg(tok(buff)-1) ? STR_DUP(buff) : NULL;
            ret[rsz]->line = line;
            ret[rsz]->file = file;
            rsz++;
            buff[bp=0]=0;
            checkop:
                if(op(*si)) {
                    ret = realloc(ret, (rsz+1)*TOKEN_SIZE);
                    ret[rsz] = TOKEN_NEW();
                    ret[rsz]->id = op(*si)-1;
                    ret[rsz]->line = line;
                    ret[rsz]->file = file;
                    ret[rsz]->value = NULL;
                    rsz++;
                }
            continue;
        }
        buff[bp++] = *si;
        buff[bp] = 0;
    }

    *lsz = rsz;
    return ret;
}

const char *id_type(int id_t) {
    switch(id_t) {
    case t_root:         return "root";
    case t_exit:         return "exit";
    case t_int:          return "int constant";
    case t_hex:          return "hex constant";
    case t_iden:         return "identifier";
    case t_label:        return "label";
    case t_jump:         return "jump";
    case t_register:     return "register";
    case t_equ:          return "=";
    case t_if:           return "if";
    case t_macro:        return "macro";
    case t_mend:         return "macro_end";
    case t_include:      return "include";
    case t_str_lit:      return "string literal";
    case t_start:        return "scope start";
    case t_end:          return "scope end";
    case t_declare:      return "declare";
    case t_char_type:    return "char type";
    case t_short_type:   return "short type";
    case t_int_type:     return "int type";
    case t_long_type:    return "long type";
    case t_pointer_type: return "pointer type";
    case t_set:          return "set";
    case t_asm:          return "asm";
    case t_add:          return "add";
    case t_sub:          return "sub";
    case t_mul:          return "mul";
    case t_div:          return "div";
    case t_alloc:        return "alloc";
    case t_get:          return "geta";
    case t_seta:         return "seta";
    case t_or:           return "or";
    case t_and:          return "and";
    case t_xor:          return "xor";
    case t_not:          return "not";
    case t_while:        return "while";
    case t_do:           return "do";
    case t_stop:         return "stop";
    case t_argset:       return "argset";
    case t_argend:       return "argend";
    case t_argget:       return "argget";
    default:             return "(null)";
    }
}

int reptok(struct token t, int n) {
    printf("%d| (%s : %s)\n", n, id_type(t.id), tokhasarg(t.id) ? t.value : "NULL");
    return 0;
}

