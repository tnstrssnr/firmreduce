#ifndef NODE_CONTAINER_H
#define NODE_CONTAINER_H


#include <libfirm/firm.h>

typedef int select_func(const ir_node* node);

typedef struct ir_node_container {

    select_func* func;
    ir_node** nodes;
    int nodes_n;

} ir_node_container;

void collect_nodes(ir_graph* irg, ir_node_container* container);

#endif