#include "main.h"
char *compile_expression(struct AST *a, struct scope_node *scope);


char *compile_expression(struct AST *a, struct scope_node *scope) {
    char *ret;
    const char *template, *tmp, *tmp2, *tmp3;
    static int expr_label_num = 0;
    int i;
    switch(a->id) {
    case t_int:
        template = "  mov rax, %s\n";
        ret = malloc(strlen(template) + strlen(a->value)+1);
        sprintf(ret, template, a->value);
        break;
    case t_char:
        template = "  mov rax, '%s'\n";
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
            for(i = 0; i < scope->variable_length; i++) {
                if(!strcmp(scope->variables[i].name, a->value)) {
                    template = "  mov rax, [rsp+%d] ; var %s ;\n";
                    ret = malloc(strlen(template) + 21);
                    sprintf(ret, template, scope->variables[i].offset,
                        scope->variables[i].name);
                    i = -1;
                    break;
                }
            }
            if(i = -1) break;
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
        tmp  = compile_expression(a->children[0], scope);
        tmp2 = compile_expression(a->children[1], scope);
        ret  = malloc(strlen(template) + strlen(tmp) + strlen(tmp2) + 20);
        sprintf(ret, template, tmp, tmp2, expr_label_num, expr_label_num,
            expr_label_num, expr_label_num);
        expr_label_num++;
        break;
    default: ret = NULL;
    }
    return ret;
}

char *compile_exit(struct AST *a, struct scope_node *scope) {
    char *ret, *_expr;
    const char *exit_template;
    if(a->id != t_exit) return NULL;
    exit_template =
        "  ; syscall exit ;\n"
        "  add rsp, %d\n"
        "%s"
        "  mov rdi, rax\n"
        "  mov rax, 60\n"
        "  syscall\n";
    _expr = compile_expression(a->children[0], scope);
    ret = malloc(strlen(exit_template) + strlen(_expr)+1);
    sprintf(ret, exit_template, scope->stack_size, _expr);
    free(_expr);
    return ret;
}

char *compile_asm(struct AST *a, struct scope_node *scope) {
    char *ret;
    if(a->id != t_asm) return NULL;
    ret = STR_DUP(a->children[0]->value);
    ret = realloc(ret, strlen(ret)+1);
    ret[strlen(ret)]='\n';
    ret[strlen(ret)+1]=0;
    return ret;
}

char *compile_label(struct AST *a, struct scope_node *scope) {
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

char *compile_jump(struct AST *a, struct scope_node *scope) {
    char *ret;
    const char *jump_template;
    if(a->id != t_jump) return NULL;
    jump_template =
        "  ; jump ;\n"
        "  add rsp, %d\n"
        "  jmp %s\n";
    ret = malloc(strlen(jump_template) + strlen(a->children[0]->value));
    sprintf(ret, jump_template, scope->stack_size, a->children[0]->value);
    return ret;
}

char *compile_jump_if(struct AST *a, struct scope_node *scope) {
    char *ret, *_expr;
    const char *jump_if_template;
    static int label_if = 0;
    if(a->id != t_if) return NULL;
    jump_if_template =
        "%s\n"
        "  ; jump if ;\n"
        "  cmp rax, 0\n"
        "  jz .IL%d\n"
        "  add rsp, %d\n"
        "  jmp %s\n"
        ".IL%d:\n";
    _expr = compile_expression(a->children[0], scope);
    ret = malloc(strlen(jump_if_template) + strlen(a->children[1]->value) +
        strlen(_expr) + 41);
    sprintf(ret, jump_if_template, _expr, label_if, scope->stack_size, a->children[1]->value, label_if);
    label_if++;
    return ret;
}

char *compile_set(struct AST *a, struct scope_node *scope) {
    char *ret, *_expr;
    struct variable var;
    const char *set_template, *word, *reg;
    int i;
    if(a->id != t_set) return NULL;
    set_template = "%s\n"
                   "  ; set ;\n"
                   "  mov %s [rsp+%d], %s\n";
    for(i = 0; i < scope->variable_length; i++) {
        if(!strcmp(scope->variables[i].name, a->children[0]->value))
            var = scope->variables[i];
    }
    reg = reg_from_size(var.size);
    word = word_from_size(var.size);
    _expr = compile_expression(a->children[1], scope);
    ret = malloc(strlen(set_template) + strlen(_expr) + strlen(word) + 21);
    sprintf(ret, set_template, _expr, word, var.offset, reg);
    return ret;
}

char *compile_register(struct AST *a, struct scope_node *scope) {
    char *ret, *_expr;
    const char *register_template, *reg_which;
    if(a->id != t_register) return NULL;
    register_template =
        "  ; register ;\n"
        "%s"
        "  mov %s\n";
    _expr = compile_expression(a->children[1], scope);
    if(!strcmp(a->children[0]->value, "A")) reg_which = "byte ch, al";
    if(!strcmp(a->children[0]->value, "B")) reg_which = "byte cl, al";
    if(!strcmp(a->children[0]->value, "X")) reg_which = "word cx, ax";
    if(!strcmp(a->children[0]->value, "E")) reg_which = "dword ecx, eax";
    ret = malloc(strlen(register_template) + strlen(_expr) + strlen(reg_which)+1);
    sprintf(ret, register_template, _expr, reg_which);
    return ret;
}

char *compile_statement(struct AST *a, struct scope_node **scope);

char *compile_scope(struct AST *a, struct scope_node *scope) {
    struct scope_node *new_scope;
    char *ret, *tmp;
    int offset;
    const char *template;
    if(a->id != t_start) return NULL;

    ret = calloc(1,1);
    offset = 0;

    new_scope = scope_from_ast(a, scope);
    if(new_scope->variable_length > 0) {
        template = "  sub rsp, %d\n"
                   "%s\n"
                   "  add rsp, %d\n";
    } else template = "%s\n";

    while(1) {
        if(offset > a->child_num-1) break;
        if(tmp=compile_statement(a->children[offset], &new_scope)) {
            if(tmp == IGNORE) {
                offset++;
                continue;
            }
            ret = realloc(ret, strlen(ret) + strlen(tmp) + 1);
            strcpy(ret + strlen(ret), tmp);
            free(tmp);
            offset++;
        }
    }

    tmp = malloc(strlen(ret) + strlen(template) + 41);
    if(new_scope->variable_length > 0) {
        sprintf(tmp, template, new_scope->stack_size, ret, new_scope->stack_size);
        ret = realloc(ret, strlen(tmp)+1);
        strcpy(ret, tmp);
        free(tmp);
    }
       
    return ret;
}

char *compile_declaration(struct AST *a, struct scope_node *scope) {
    if(a->id != t_declare) return NULL;
    return IGNORE;
}

#define STATEMENT_LEN 9
char *compile_statement(struct AST *a, struct scope_node **scope) {
    char *statements[STATEMENT_LEN] = { compile_label(a, *scope), compile_exit(a, *scope),
        compile_jump(a, *scope), compile_register(a, *scope), compile_jump_if(a, *scope),
        compile_scope(a, *scope), compile_declaration(a, *scope), compile_set(a, *scope),
        compile_asm(a, *scope) };
    int i;
    for(i=0; i < STATEMENT_LEN; i++) {
        if(statements[i] == IGNORE) return IGNORE;
        if(statements[i] != NULL) return statements[i];
    }
    return NULL;
}

char *compile(struct AST *a) {
    char *ret, *tmp;
    int offset;
    const char *code_template, *generic_template;
    struct scope_node *scope;
    code_template =
        "section .text\n"
        "  global _start\n\n"
        "%s\n";
    
    ret = calloc(1,1);
    offset = 0;

    scope = scope_from_ast(a, NULL);
    if(scope->variable_length > 0) {
        generic_template = "  sub rsp, %d\n";
        tmp = malloc(strlen(generic_template) + 21);
        sprintf(tmp, generic_template, scope->stack_size);
        ret = realloc(ret, strlen(tmp)+1);
        strcpy(ret, tmp);
        free(tmp);
    }
    while(1) {
        if(offset > a->child_num-1) break;
        if(tmp=compile_statement(a->children[offset], &scope)) {
            if(tmp == IGNORE) {
                offset++;
                continue;
            }
            ret = realloc(ret, strlen(ret)+1 + strlen(tmp));
            strcpy(ret+strlen(ret), tmp);
            free(tmp);
            offset++;
        } else break;
    }
    if(scope->variable_length > 0) {
        generic_template = "  add rsp, %d\n";
        tmp = malloc(strlen(generic_template) + 21);
        sprintf(tmp, generic_template, scope->stack_size);
        ret = realloc(ret, strlen(ret) + strlen(tmp) + 1);
        strcpy(ret+strlen(ret), tmp);
        free(tmp);
    }
    tmp = malloc(strlen(ret) + strlen(code_template));
    sprintf(tmp, code_template, ret);
    ret = realloc(ret, strlen(tmp)+1);
    strcpy(ret, tmp);
    free(tmp);
    return ret;
}
