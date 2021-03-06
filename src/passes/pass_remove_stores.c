#include <firm.h>
#include <pass_utils.h>

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

/*
 * Remove all Store nodes
 */

int pass_remove_stores_individual(ir_graph* irg, void* data) {

    ir_node_container* container = new_container(is_Store);
    collect_nodes(irg, container);

    if(container->nodes_n == 0) {
        free(container);
        return 0;
    }

    edges_activate(irg);

    ir_node* node = container->nodes[rand() % container->nodes_n];

    ir_node* mem_input = get_Store_mem(node);

    ir_node* mem_output;
    foreach_out_edge(node, edge) {
        if(is_Proj(get_edge_src_irn(edge))) {
            mem_output = get_edge_src_irn(edge);
        }
    }

    exchange(mem_output, mem_input);
    edges_deactivate(irg);
    free(container);
    return 1;
}

int pass_remove_stores(ir_graph* irg, void* data) {

    ir_node_container* container = new_container(is_Store);
    collect_nodes(irg, container);

    if(container->nodes_n == 0) {
        free(container);
        return 0;
    }

    edges_activate(irg);
    int* random_order = get_shuffle(container->nodes_n);

    for(int i = 0; i < container->nodes_n; i++) {

        ir_node* node = container->nodes[random_order[i]];
        ir_node* mem_input = get_Store_mem(node);
        ir_node* mem_output;

        foreach_out_edge(node, edge) {
            mem_output = get_edge_src_irn(edge);
        }
        exchange(mem_output, mem_input);
    }
    edges_deactivate(irg);
    return 1;
}

int main(int argc, char** argv) {
    if(argc != 6) {
        fprintf(stderr, "Unexpected number of arguments on call %s\n", argv[0]);
        exit(-1);
    }

    char* import_file = argv[1];
    char* dump = argv[2];
    int reduce_conservatively = atoi(argv[3]);
    char* irg_ident = argv[4];
    int seed = atoi(argv[5]);
    srand(seed);

    return (reduce_conservatively) ? apply_pass(import_file, dump, &pass_remove_stores_individual, irg_ident) : apply_pass(import_file, dump, &pass_remove_stores, irg_ident);
    
}