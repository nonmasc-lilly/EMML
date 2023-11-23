#include "main.h"


#define IFNCMP(a, b) if(!strcmp(a, b))
#define ELNCMP(a, b) else IFNCMP(a, b)
int tok(char *b) {
    int ret, isnum;
    char *si;
    IFNCMP(b, "") ret = -1;
    ELNCMP(b, "EXIT") ret = t_exit;
    ELNCMP(b, "LABEL") ret = t_label;
    else {
        for(isnum=1,si=b; *si && isnum; si++)
            isnum = isdigit(*si);
        ret = isnum ? t_int : t_iden;
    }
    return ret+1;
}
#undef IFNCMP
#undef ELNCMP

int tokhasarg(int tok) {
    int arg_toks[] = { t_int, t_iden },
        i = 0;
    for(i=0; i < sizeof(arg_toks); i++)
        if(tok == arg_toks[i]) return 1;
    return 0;
}

char *STR_DUP(char *a) {
    char *ret;
    ret = malloc(strlen(a));
    strcpy(ret, a);
    return ret;
}

struct token **lex(char *s, int *lsz) {
    char buff[1024] = {0};
    int bp, rsz;
    char *si;
    struct token **ret;

    ret = malloc(1);
    rsz = 0;
    bp=0;

    for(si=s; *si; si++) {
        if(isspace(*si)) {
            if(buff[0] == 0) continue;
            if(!tok(buff)) continue;
            ret = realloc(ret, (rsz+1)*TOKEN_SIZE);
            ret[rsz] = TOKEN_NEW();
            ret[rsz]->id = tok(buff)-1;
            ret[rsz]->value = tokhasarg(tok(buff)-1) ? STR_DUP(buff) : NULL;
            rsz++;
            buff[bp=0]=0;
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
    case t_root: return "root";
    case t_exit: return "exit";
    case t_int: return "int constant";
    case t_hex: return "hex constant";
    case t_iden: return "identifier";
    case t_label: return "label";
    }
}

int reptok(struct token t, int n) {
    printf("%d| (%s : %s)\n", n, id_type(t.id), tokhasarg(t.id) ? t.value : "NULL");
    return 0;
}

