#include "main.h"

int literal_types(int literal) {
    switch(literal) {
    case t_char: return t_char_type;
    case t_int: return t_int_type;
    case t_str_lit: return t_pointer_type;
    }
}

int size_from_type(int type) {
    switch(type) {
    case t_char_type:    return 1;
    case t_short_type:   return 2;
    case t_int_type:     return 4;
    case t_long_type:    return 8;
    case t_pointer_type: return 8;
    default: return -1;
    }
}

const char *word_from_size(int size) {
    switch(size) {
    case 1: return "byte";
    case 2: return "word";
    case 4: return "dword";
    case 8: return "qword";
    }
}

const char *reg_from_size(int size) {
    switch(size) {
    case 1: return "al";
    case 2: return "ax";
    case 4: return "eax";
    case 8: return "rax";
    }
}

struct scope_node *scope_new() {
    struct scope_node *ret;
    ret = malloc(SCOPE_SIZE);
    ret->variables = malloc(1);
    ret->variable_length = 0;
    ret->stack_size = 0;
}

void scope_del(struct scope_node *scope) {
    if(scope->child != NULL)
        scope_del(scope->child);
    scope->parent->child = NULL;
}

struct scope_node *scope_from_ast(struct AST *a, struct scope_node *parent) {
    struct SCOPE *node = SCOPE_NEW();
    int i, offset;
    for(i = 0; i < a->child_num; i++) {
        if(a->children[i]->id == t_declare) {
            node->variable_length += 1;
            node->variables = realloc(node->variables,
                node->variable_length*VARIABLE_SIZE);
            node->variables[node->variable_length-1].size =
                size_from_type(a->children[i]->children[0]->id);
            node->variables[node->variable_length-1].name =
                a->children[i]->children[1]->value;
        }
    }
    offset = 0;
    for(i = 0; i < node->variable_length; i++)
        node->variables[i].offset = (offset += node->variables[i].size);
    node->stack_size = offset + (8-(offset % 8));
    if(parent != NULL) SCOPE_ADD(parent, node);
    return node;
}


