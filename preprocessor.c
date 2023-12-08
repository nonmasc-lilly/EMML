#include "main.h"

static int process_macro(struct token_link *link, struct token **lexed, int lsz,
        int idx) {
    int i;
    if(lexed[idx]->value == NULL) return 0;
    link->id = lexed[idx]->value;
    for(link->tsz = 0; lexed[idx+1+link->tsz]->id != t_mend; link->tsz++);
    link->tokens = malloc(link->tsz * sizeof(struct token*));
    for(i = 0; i < link->tsz; i++)
        link->tokens[i] = lexed[i+1+idx];
    return i;
}

struct token **preprocess(struct token **lexed, int lsz, int *rsz,
        struct token_link *link_prev, int lksz) {
    struct token **ret, **file;
    struct token_link *links;
    const char *identity;
    int i, j, k,
        tmp, psz, fsz,
        changed;

    if(link_prev != NULL) links = link_prev;

    changed = 0;

    ret   = malloc(1);
    links = link_prev == NULL ? malloc(1) : link_prev;
    *rsz  = 0;
    
    for(i = 0; i < lsz; i++) {
        switch(lexed[i]->id) {
        case t_macro:
            links = realloc(links, (++lksz) * sizeof(struct token_link));
            if(tmp=process_macro(links+lksz-1, lexed, lsz, i+1)) { i+=tmp+2; continue; }
            break;
        }
        tmp=0;
        if(lexed[i]->value != NULL) for(j = 0; j < lksz; j++)
            if(!strcmp(lexed[i]->value, links[j].id)) {
                ret = realloc(ret, ((*rsz)+links[j].tsz)*sizeof(struct token*));
                for(k = 0; k < links[j].tsz; k++)
                    TOKEN_FROM(links[j].tokens[k], ret[k+(*rsz)]);
                *rsz += links[j].tsz;
                changed = 1;
                tmp = 1;
                break;
            }
        if(tmp) continue;
        ret       = realloc(ret, ((*rsz)+1)*sizeof(struct token*));
        ret[*rsz] = lexed[i];
        (*rsz)++;
        continue;
    }
    if(changed) {
        ret = preprocess(ret, *rsz, &psz, links, lksz);
        *rsz = psz;
    }
    return ret;
}
