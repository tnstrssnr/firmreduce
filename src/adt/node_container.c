#include <stdlib.h>
#include <string.h>

#include <node_container.h>

void find_nodes_walk(ir_node* node, void* data) {

    ir_node_container* container = (ir_node_container*) data;

    if((sizeof(container->nodes)) <= container->nodes_n) {
        ir_node** cond_nodes_new = malloc(2*sizeof(container->nodes));
        memcpy(cond_nodes_new, container->nodes, sizeof(ir_node*)*container->nodes_n);
        free(container->nodes);
        container->nodes = cond_nodes_new;
    }

    if((container->func)(node)) {
        container->nodes[container->nodes_n] = node;
        container->nodes_n++;
    }

}


void collect_nodes(ir_graph* irg, ir_node_container* container) {

    container->nodes = malloc(10*sizeof(ir_node*));
    container->nodes_n = 0;
    irg_walk_graph(irg, find_nodes_walk, NULL, container);

}