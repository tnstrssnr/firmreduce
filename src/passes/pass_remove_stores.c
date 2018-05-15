#include <firm.h>
#include <node_container.h>

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

void pass_remove_stores(ir_graph* irg, void* data) {

    ir_node_container* container = new_container(is_Store);
    collect_nodes(irg, container);

    if(container->nodes_n > 0) {
        edges_activate(irg);

        srand(time(NULL));
        ir_node* node = container->nodes[rand() % container->nodes_n];

        ir_node* mem_input = get_Store_mem(node);

        ir_node* mem_output;
        foreach_out_edge(node, edge) {
            if(is_Proj(get_edge_src_irn(edge))) {
                mem_output = get_edge_src_irn(edge);
            }
        }

        edges_reroute(mem_output, mem_input);
        edges_deactivate(irg);
    }
    free(container);
}