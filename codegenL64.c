#include "main.h"
char *compile_expression(struct AST *a);


char *compile_expression(struct AST *a) {
    char *ret;
    const char *template, *tmp, *tmp2, *tmp3;
    static int expr_label_num = 0;
    switch(a->id) {
    case t_int:
        template = "  mov rax, %s\n";
        ret = malloc(strlen(template) + strlen(a->value)+1);
        sprintf(ret, template, a->value);
        break;
    case t_iden:
        template = "  xor rax, rax\n"
                   "  mov %s, %s\n";
             if(!strcmp(a->value, "A")) (tmp2 = "al",  tmp = "ch");
        else if(!strcmp(a->value, "B")) (tmp2 = "al",  tmp = "cl");
        else if(!strcmp(a->value, "X")) (tmp2 = "ax",  tmp = "cx");
        else if(!strcmp(a->value, "E")) (tmp2 = "eax", tmp = "ecx");
        else {
            template = "  mov rax, [%s]\n";
            tmp = a->value;
            ret = malloc(strlen(template) + strlen(tmp)+1);
            sprintf(ret, template, tmp);
            break;
        }
        ret = malloc(strlen(template) + strlen(tmp) + strlen(tmp2) + 1);
        sprintf(ret, template, tmp2, tmp);
        break;
    case t_equ:
        template = "%s\n"
                   "  mov rbx, rax\n"
                   "%s\n"
                   "  sub rbx, rax\n"
                   "  cmp rbx, 0\n"
                   "  jz .EL%d\n"
                   "  xor rax, rax\n"
                   "  jmp .EL%desc\n"
                   ".EL%d:\n"
                   "  mov rax, 1\n"
                   ".EL%desc:\n";
        tmp  = compile_expression(a->children[0]);
        tmp2 = compile_expression(a->children[1]);
        ret  = malloc(strlen(template) + strlen(tmp) + strlen(tmp2) + 20);
        sprintf(ret, template, tmp, tmp2, expr_label_num, expr_label_num,
            expr_label_num, expr_label_num);
        expr_label_num++;
        break;
    default: ret = NULL;
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
    free(_expr);
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

char *compile_jump(struct AST *a) {
    char *ret;
    const char *jump_template;
    if(a->id != t_jump) return NULL;
    jump_template =
        "  ; jump ;\n"
        "  jmp %s\n";
    ret = malloc(strlen(jump_template) + strlen(a->children[0]->value));
    sprintf(ret, jump_template, a->children[0]->value);
    return ret;
}

char *compile_jump_if(struct AST *a) {
    char *ret, *_expr;
    const char *jump_if_template;
    static int label_if = 0;
    if(a->id != t_if) return NULL;
    jump_if_template =
        "%s\n"
        "  ; jump if ;\n"
        "  cmp rax, 0\n"
        "  jz .IL%d\n"
        "  jmp %s\n"
        ".IL%d:\n";
    _expr = compile_expression(a->children[0]);
    ret = malloc(strlen(jump_if_template) + strlen(a->children[1]->value) +
        strlen(_expr) + 41);
    sprintf(ret, jump_if_template, _expr, label_if, a->children[1]->value, label_if);
    label_if++;
    return ret;
}

char *compile_register(struct AST *a) {
    char *ret, *_expr;
    const char *register_template, *reg_which;
    if(a->id != t_register) return NULL;
    register_template =
        "  ; register ;\n"
        "%s"
        "  mov %s\n";
    _expr = compile_expression(a->children[1]);
    if(!strcmp(a->children[0]->value, "A")) reg_which = "byte ch, al";
    if(!strcmp(a->children[0]->value, "B")) reg_which = "byte cl, al";
    if(!strcmp(a->children[0]->value, "X")) reg_which = "word cx, ax";
    if(!strcmp(a->children[0]->value, "E")) reg_which = "dword ecx, eax";
    ret = malloc(strlen(register_template) + strlen(_expr) + strlen(reg_which)+1);
    sprintf(ret, register_template, _expr, reg_which);
    return ret;
}

#define STATEMENT_LEN 6
char *compile_statement(struct AST *a) {
    char *statements[STATEMENT_LEN] = { compile_label(a), compile_exit(a),
        compile_jump(a), compile_register(a), compile_jump_if(a) };
    int i;
    for(i=0; i < STATEMENT_LEN; i++) {
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
