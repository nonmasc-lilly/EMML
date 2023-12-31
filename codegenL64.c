#include "main.h"
char *compile_expression(struct AST *a, struct scope_node *scope);


char *compile_expression(struct AST *a, struct scope_node *scope) {
    char *ret;
    const char *template,
        *argreg[6] = {"rdi", "rsi", "rdx", "r10", "r8", "r9"};
    char *tmp, *tmp2, *tmp3;
    struct variable var;
    static int expr_label_num = 0;
    int i;
    switch(a->id) {
    case t_int:
        if(!atoi(a->value)) {
            template = "  xor rax, rax";
            ret = calloc(1,strlen(template));
            strcpy(ret, template);
        }
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
                    template = "  mov %s, [rsp+%d] ; var %s ;\n";
                    ret = malloc(strlen(template) + 21);
                    sprintf(ret, template, reg_from_size(scope->variables[i].size),
                        scope->variables[i].offset, scope->variables[i].name);
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
                   "  xor rcx, rcx\n"
                   "  mov rdx, 1\n"
                   "  cmp rax, rbx\n"
                   "  cmove rax, rdx\n"
                   "  cmovne rax, rcx\n";
        tmp  = compile_expression(a->children[0], scope);
        tmp2 = compile_expression(a->children[1], scope);
        ret  = malloc(strlen(template) + strlen(tmp) + strlen(tmp2) + 20);
        sprintf(ret, template, tmp, tmp2);
        break;
    case t_add:
        template = "%s\n"
                   "  mov rbx, rax\n"
                   "%s\n"
                   "  add rax, rbx\n";
        tmp2 = compile_expression(a->children[0], scope);
        tmp  = compile_expression(a->children[1], scope);
        ret  = malloc(strlen(template) + strlen(tmp) + strlen(tmp2) + 20);
        sprintf(ret, template, tmp, tmp2);
        break;
    case t_sub:
        template = "%s\n"
                   "  mov rbx, rax\n"
                   "%s\n"
                   "  sub rax, rbx\n";
        tmp2 = compile_expression(a->children[0], scope);
        tmp  = compile_expression(a->children[1], scope);
        ret  = malloc(strlen(template) + strlen(tmp) + strlen(tmp2) + 21);
        sprintf(ret, template, tmp, tmp2);
        break;
    case t_mul:
        template = "%s\n"
                   "  mov rbx, rax\n"
                   "%s\n"
                   "  push rdx\n"
                   "  imul rbx\n"
                   "  pop rdx\n";
        tmp2 = compile_expression(a->children[0], scope);
        tmp  = compile_expression(a->children[1], scope);
        ret  = malloc(strlen(template) + strlen(tmp) + strlen(tmp2) + 21);
        sprintf(ret, template, tmp, tmp2);
        break;
    case t_div:
        template = "%s\n"
                   "  mov rbx, rax\n"
                   "%s\n"
                   "  push rdx\n"
                   "  idiv rbx\n"
                   "  pop rdx\n";
        tmp2 = compile_expression(a->children[0], scope);
        tmp  = compile_expression(a->children[1], scope);
        ret  = malloc(strlen(template) + strlen(tmp) + strlen(tmp2) + 21);
        sprintf(ret, template, tmp, tmp2);
        break;
    case t_or:
        template = "%s\n"
                   "  mov rbx, rax\n"
                   "%s\n"
                   "  or rax, rbx\n";
        tmp2 = compile_expression(a->children[0], scope);
        tmp  = compile_expression(a->children[1], scope);
        ret  = malloc(strlen(template) + strlen(tmp) + strlen(tmp2) + 1);
        sprintf(ret, template, tmp, tmp2);
        break;
    case t_and:
        template = "%s\n"
                   "  mov rbx, rax\n"
                   "%s\n"
                   "  and rax, rbx\n";
        tmp2 = compile_expression(a->children[0], scope);
        tmp  = compile_expression(a->children[1], scope);
        ret  = malloc(strlen(template) + strlen(tmp) + strlen(tmp2) + 1);
        sprintf(ret, template, tmp, tmp2);
        break;
    case t_xor:
        template = "%s\n"
                   "  mov rbx, rax\n"
                   "%s\n"
                   "  xor rax, rbx\n";
        tmp2 = compile_expression(a->children[0], scope);
        tmp  = compile_expression(a->children[1], scope);
        ret  = malloc(strlen(template) + strlen(tmp) + strlen(tmp2) + 1);
        sprintf(ret, template, tmp, tmp2);
        break;
    case t_not:
        template = "%s\n"
                   "  not rax\n";
        tmp  = compile_expression(a->children[0], scope);
        ret  = malloc(strlen(template) + strlen(tmp) + 1);
        sprintf(ret, template, tmp);
        break;
    case t_get:
        template = "%s\n"
                   "  mov rbx, [rsp+%d+rax*%d]\n"
                   "  mov rax, rbx\n";
        for(i = 0; i < scope->variable_length; i++)
            if(!strcmp(scope->variables[i].name, a->children[0]->value))
                var = scope->variables[i];
        tmp = compile_expression(a->children[1], scope);
        ret = malloc(strlen(template) + strlen(tmp) + 21);
        sprintf(ret, template, tmp, var.offset, size_from_type(var.subtype));
        break;
    case t_ref:
        template = "lea rax, [rsp+%d]\n";
        for(i = 0; i < scope->variable_length; i++)
            if(!strcmp(scope->variables[i].name, a->children[0]->value))
                var = scope->variables[i];
        ret = malloc(strlen(template) + 21);
        sprintf(ret, template, var.offset);
        break;
    case t_deref:
        template = "%s\n"
                   "  mov rbx, rax\n"
                   "  mov rax, [rbx]\n";
        tmp = compile_expression(a->children[0], scope);
        ret = malloc(strlen(template) + strlen(tmp) + 1);
        sprintf(ret, template, tmp);
        break;
    case t_argget:
        template = "  mov rax, %s\n";
        ret = malloc(strlen(template) + 5);
        sprintf(ret, template, argreg[atoi(a->children[0]->value)]);
        break;
    case t_run:
        template = "call %s\n";
        ret = malloc(strlen(template) + strlen(a->children[0]->value) + 1);
        sprintf(ret, template, a->children[0]->value);
        return ret;
    default: ret = NULL;
    }
    return ret;
}

char *compile_argset(struct AST *a, struct scope_node *scope) {
    char *ret, *_expr, *tmp;
    const char *regs[] = { "rdi", "rsi", "rdx", "r10", "r8", "r9" };
    const char *template;
    int i;
    if(a->id != t_argset) return NULL;
    ret = calloc(1,strlen("  ; argset ;\n")+1);
    strcat(ret, "  ; argset ;\n");
    template = "%s  mov %s, rax\n";
    for(i=0; i < a->child_num; i++) {
        _expr = compile_expression(a->children[i], scope);
        tmp = malloc(strlen(_expr) + strlen(template) + 1);
        sprintf(tmp, template, _expr, regs[i]);
        ret = realloc(ret, strlen(ret) + strlen(tmp) + 1);
        strcat(ret, tmp);
        free(tmp);
        free(_expr);
    }
    return ret;
}

char *compile_exit(struct AST *a, struct scope_node *scope) {
    char *ret, *_expr;
    const char *exit_template;
    if(a->id != t_exit) return NULL;
    exit_template =
        "  ; syscall exit ;\n"
        "%s\n"
        "  add rsp, %d\n"
        "  mov rdi, rax\n"
        "  mov rax, 60\n"
        "  syscall\n";
    _expr = compile_expression(a->children[0], scope);
    ret = malloc(strlen(exit_template) + strlen(_expr)+1);
    sprintf(ret, exit_template, _expr, scope->stack_size);
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

char *compile_seta(struct AST *a, struct scope_node *scope) {
    char *ret, *_expr;
    struct variable var;
    const char *set_template, *word, *reg, *index;
    int i;
    if(a->id != t_seta) return NULL;
    set_template = "; set a ;\n"
                   "%s\n"
                   "  mov rbx, rax\n"
                   "%s\n"
                   "  mov %s [rsp+%d+rbx*%d], %s\n";
    for(i = 0; i < scope->variable_length; i++)
        if(!strcmp(scope->variables[i].name, a->children[0]->value))
            var = scope->variables[i];
    _expr = compile_expression(a->children[2], scope);
    reg = reg_from_size(size_from_type(var.subtype));
    word = word_from_size(size_from_type(var.subtype));
    index = compile_expression(a->children[1], scope);
    ret = malloc(strlen(set_template) + strlen(_expr) + strlen(word) + strlen(index) + 41);
    sprintf(ret, set_template, index, _expr, word, var.offset,
        size_from_type(var.subtype), reg);
    return ret;
}

char *compile_set(struct AST *a, struct scope_node *scope) {
    char *ret, *_expr;
    struct variable var;
    const char *set_template, *word, *reg, *index;
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

char *compile_alloc(struct AST *a, struct scope_node *scope) {
    if(a->id == t_alloc) return IGNORE;
    return NULL;
}

char *compile_statement(struct AST *a, struct scope_node **scope);

char *compile_while(struct AST *a, struct scope_node *scope) {
    struct scope_node *new_scope;
    char *ret, *_expr, *tmp;
    int offset;
    const char *template;

    static int wl = -1;
    if(a->id != t_while) return NULL;
    wl++;
    template = "  ; while ;\n"
               "  .w%d:"
               "%s\n"
               "  or rax, rax\n"
               "  jz .w%do\n"
               "%s\n"
               "  jmp .w%d\n"
               "  .w%do:\n";
    _expr = compile_expression(a->children[0], scope);
    ret = calloc(1,1);
    offset = 1;
    while(1) {
        if(offset > a->child_num-1) break;
        if(tmp=compile_statement(a->children[offset], &scope)) {
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

    tmp = malloc(strlen(ret) + strlen(template) + strlen(_expr) + 81);
    sprintf(tmp, template, wl, _expr, wl, ret, wl, wl);
    ret = realloc(ret, strlen(tmp)+1);
    strcpy(ret, tmp);
    free(tmp);
    return ret;
}

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
        template = "  ; scope ;\n"
                   "  sub rsp, %d\n"
                   "%s\n"
                   "  add rsp, %d\n";
    } else template = "; scope ;\n%s\n";

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

char *compile_subrt(struct AST *a, struct scope_node *scope) {
    char *ret, *_scope;
    const char *template;
    if(a->id != t_subrt) return NULL;
    template = "%s:\n"
               "%s\n"
               "  mov rax, 0\n"
               "  ret\n";
    _scope = compile_scope(a->children[2], scope);
    ret = calloc(1,strlen(a->children[1]->value) + strlen(_scope) + strlen(template)
        + 1);
    sprintf(ret, template, a->children[1]->value, _scope);
    free(_scope);
    return ret;
}

char *compile_return(struct AST* a, struct scope_node *scope) {
    char *ret, *_expr;
    const char *template;
    if(a->id != t_return) return NULL;
    template = "%s\n"
               "  ret";

    _expr = compile_expression(a->children[0], scope);
    ret = calloc(1,strlen(template)+strlen(_expr)+1);
    sprintf(ret, template, _expr);
    free(_expr);
    return ret;
    
}

char *compile_declaration(struct AST *a, struct scope_node *scope) {
    if(a->id != t_declare) return NULL;
    return IGNORE;
}

#define STATEMENT_LEN 15
char *compile_statement(struct AST *a, struct scope_node **scope) {
    char *statements[STATEMENT_LEN] = { compile_label(a, *scope), compile_exit(a, *scope),
        compile_jump(a, *scope), compile_register(a, *scope), compile_jump_if(a, *scope),
        compile_scope(a, *scope), compile_declaration(a, *scope), compile_set(a, *scope),
        compile_asm(a, *scope), compile_alloc(a, *scope), compile_seta(a, *scope),
        compile_while(a, *scope), compile_argset(a, *scope), compile_subrt(a, *scope),
        compile_return(a, *scope), };
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
