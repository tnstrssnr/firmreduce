#include <firm.h>
#include <pass_utils.h>

#include <stdlib.h>
#include <time.h>

/*
 * Remove all Alloc nodes, that only have a memory out-edge
 */

int is_Alloc_no_out_edge(const ir_node* node) {
    return is_Alloc(node) && (1 == get_irn_n_edges(node));
}

void remove_alloc(ir_graph* irg, ir_node* node) {

    
    ir_node* mem_input = get_Alloc_mem(node);
    ir_node* mem_output;

    // look for memory state after the call happened
    foreach_out_edge(node, edge) {
        if(is_Proj(get_edge_src_irn(edge))) {
            mem_output = get_edge_src_irn(edge);
        }
    }

    exchange(mem_output, mem_input);
}

int pass_remove_allocs_individual(ir_graph* irg, void* data) {
    edges_activate(irg);
    ir_node_container* container = new_container(is_Alloc_no_out_edge);
    collect_nodes(irg, container);

    if(container->nodes_n == 0) return 0;

    ir_node* node = container->nodes[rand() % container->nodes_n];
    remove_alloc(irg, node);
    free(container);
    return 1;
}

int pass_remove_allocs(ir_graph* irg, void* data) {
    edges_activate(irg);
    ir_node_container* container = new_container(is_Alloc_no_out_edge);
    collect_nodes(irg, container);

    if(container->nodes_n == 0) return 0;

    int* random_order = get_shuffle(container->nodes_n);

    for(int i = 0; i < container->nodes_n; i++) {
        ir_node* node = container->nodes[random_order[i]];
        remove_alloc(irg, node);
    }
    free(container);
    free(random_order);
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


    return (reduce_conservatively) ? apply_pass(import_file, dump, &pass_remove_allocs_individual, irg_ident) : apply_pass(import_file, dump, &pass_remove_allocs, irg_ident);
}