#ifndef PASS_UTILS_H
#define PASS_UTILS_H

#include <firm.h>
#include <stdbool.h>

extern const char* TEMP_DIR;
extern const char* TEMP_IR;
extern const char* CURR_IR;

/**
 * function type that should return true, if node is relevant for the current pass
 */
typedef int select_func(const ir_node* node);

/**
 * function type, that applies a pass operation to the irg
 */
typedef int pass_func(ir_graph* irg, void* data);

typedef void opt_func(ir_graph* irg);

typedef struct ir_node_container {

    select_func* func;
    ir_node** nodes;
    int nodes_n;

} ir_node_container;

ir_node_container* new_container(select_func* func);

void collect_nodes(ir_graph* irg, ir_node_container* container);

int apply_pass(char* path, char* dump, pass_func* func, char* ident);

int* get_shuffle(int size);

bool is_valid();

int select_all(const ir_node* node);

int apply_optimization(char* file, char* dump, char* ident, opt_func* func);

ir_graph* get_irg_by_ident(const char* ident);

#endif