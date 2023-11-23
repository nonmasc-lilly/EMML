#include "main.h"

char *compile_expression(struct AST *a) {
    char *ret;
    const char *template;
    switch(a->id) {
    case t_int:
        template = "  mov rax, %s\n";
        ret = malloc(strlen(template) + strlen(a->value)+1);
        sprintf(ret, template, a->value);
        break;
    case t_iden:
        template = "  mov rax, [%s]\n";
        ret = malloc(strlen(template) + strlen(a->value)+1);
        sprintf(ret, template, a->value);
        break;
    default: return NULL;
    }
    return ret;
}

char *compile_exit(struct AST *a) {
    char *ret, *_expr;
    const char *exit_template;
    if(a->id != t_exit) return NULL;
    exit_template =
        "  ; syscall exit ;\n"
        "%s"
        "  mov rdi, rax\n"
        "  mov rax, 60\n"
        "  syscall\n";
    _expr = compile_expression(a->children[0]);
    ret = malloc(strlen(exit_template) + strlen(_expr)+1);
    sprintf(ret, exit_template, _expr);
    return ret;
}

char *compile_label(struct AST *a) {
    char *ret;
    const char *label_template;
    if(a->id != t_label) return NULL;
    label_template =
        "; label ;\n"
        "%s:\n";
    ret = malloc(strlen(label_template) + strlen(a->children[0]->value));
    sprintf(ret, label_template, a->children[0]->value);
    return ret;
}

char *compile_statement(struct AST *a) {
    char *statements[] = { compile_label(a), compile_exit(a) };
    int i;
    for(i=0; i < sizeof(statements); i++) {
        if(statements[i] != NULL) return statements[i];
    }
    return NULL;
}

char *compile(struct AST *a) {
    char *ret, *tmp;
    int offset;
    const char *code_template;
    code_template =
        "section .text\n"
        "  global _start\n\n"
        "%s\n";

    ret = calloc(1,1);
    offset = 0;
    while(1) {
        if(offset > a->child_num-1) break;
        if(tmp=compile_statement(a->children[offset])) {
            ret = realloc(ret, strlen(ret)+1 + strlen(tmp));
            strcpy(ret+strlen(ret), tmp);
            free(tmp);
            offset++;
        } else break;
    }
    tmp = malloc(strlen(ret) + strlen(code_template));
    sprintf(tmp, code_template, ret);
    ret = realloc(ret, strlen(tmp)+1);
    strcpy(ret, tmp);
    free(tmp);
    return ret;
}
