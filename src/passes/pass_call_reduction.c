#include <libfirm/firm.h>
#include <stdio.h>
#include <stdlib.h>
#include <pass_utils.h>
#include <time.h>

/**
 * Remove void function calls and function calls, where return value is not used
 */

int is_Call_void(const ir_node* node) {

    return is_Call(node) && (1 == get_irn_n_edges(node));

}

int pass_call_reduction_individual(ir_graph* irg, void* data) {

    edges_activate(irg);

    ir_node_container* container = new_container(is_Call_void);
    collect_nodes(irg, container);
    if(container->nodes_n == 0) return 0;

    srand(time(NULL));
    ir_node* node = container->nodes[rand() % container->nodes_n];
    ir_node* mem_input = get_Call_mem(node);

    ir_node* mem_output;

    // look for memory state after the call happened
    foreach_out_edge(node, edge) {
        if(is_Proj(get_edge_src_irn(edge))) {
            mem_output = get_edge_src_irn(edge);
        }
    }

    // reroute all edges to old memory state
    exchange(mem_output, mem_input);
    edges_deactivate(irg);

    collect_nodes(irg, container);
    int left_to_do = container->nodes_n;
    free(container);
    return left_to_do;
}

int pass_call_reduction(ir_graph* irg, void* data) {

    edges_activate(irg);

    ir_node_container* container = new_container(is_Call_void);
    collect_nodes(irg, container);
    if(container->nodes_n == 0) return 0;

    int* random_order = get_shuffle(container->nodes_n);

    for(int i = 0; i < container->nodes_n; i++) {  
        ir_node* node = container->nodes[random_order[i]];
        ir_node* mem_input = get_Call_mem(node);
        ir_node* mem_output;

        // look for memory state after the call happened
        foreach_out_edge(node, edge) {
            if(is_Proj(get_edge_src_irn(edge))) {
                mem_output = get_edge_src_irn(edge);
            }
        }

        // reroute all edges to old memory state
        exchange(mem_output, mem_input);
    }
    edges_deactivate(irg);
    collect_nodes(irg, container);
    int left_to_do = container->nodes_n;
    free(container);
    return left_to_do;
}

int main(int argc, char** argv) {
    if(argc != 4) {
        return -1;
    }
    return (atoi(argv[3]) == 1) ? apply_pass(argv[1], &pass_call_reduction_individual, atoi(argv[2])) : apply_pass(argv[1], &pass_call_reduction, atoi(argv[2]));
}