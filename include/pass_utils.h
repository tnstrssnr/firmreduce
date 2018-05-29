#ifndef PASS_UTILS_H
#define PASS_UTILS_H


#include <firm.h>

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

typedef struct ir_node_container {

    select_func* func;
    ir_node** nodes;
    int nodes_n;

} ir_node_container;

ir_node_container* new_container(select_func* func);

void collect_nodes(ir_graph* irg, ir_node_container* container);

int apply_pass(pass_func* func);
int apply_pass_individual(pass_func* func);

#endif