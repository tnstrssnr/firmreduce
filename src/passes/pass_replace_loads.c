#include <libfirm/firm.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pass_utils.h>

int is_load_primitive(const ir_node* node) {
    return (is_Load(node) && is_Primitive_type(get_Load_type(node)) && !is_irn_start_block_placed(node));
}

/*
 * Replace Load nodes w/ Const values
 */
void replace_load(ir_graph* irg, ir_node* node) {
    ir_tarval* val = get_mode_null(get_Load_mode(node));
    dbg_info * dbgi = get_irn_dbg_info(node);
    ir_node* const_node = new_rd_Const(dbgi, irg, val);

    ir_node* mem_input = get_Load_mem(node);
    ir_node* mem_output;

    edges_activate(irg);

    foreach_out_edge(node, edge) {
        if(is_memop(get_edge_src_irn(edge))) {
            mem_output = get_edge_src_irn(edge);
        }
    }
    exchange(mem_output, mem_input);
    exchange(node, const_node);

    edges_deactivate(irg);
}

int pass_replace_loads_individual(ir_graph* irg, void* data) {
    
    ir_node_container* container = malloc(sizeof(ir_node_container));
    container->func = is_load_primitive;
    collect_nodes(irg, container);

    if (container->nodes_n == 0) {
        free(container);
        return 0;
    }
    srand(time(NULL));
    ir_node* node = container->nodes[rand() % container->nodes_n];
    replace_load(irg, node);

    free(container);
    return 1;

}

int pass_replace_loads(ir_graph* irg, void* data) {
    
    ir_node_container* container = malloc(sizeof(ir_node_container));
    container->func = is_load_primitive;
    collect_nodes(irg, container);

    if (container->nodes_n == 0) {
        free(container);
        return 0;
    }
    int* random_order = get_shuffle(container->nodes_n);

    for(int i = 0; i < container->nodes_n; i++) {
        ir_node* node = container->nodes[random_order[i]];
        replace_load(irg, node);
    }

    free(container);
    free(random_order);
    return 1;

}

int main(int argc, char** argv) {
    char* import_file = argv[1];
    int reduce_conservatively = atoi(argv[2]);
    char* irg_ident = argv[3];

    return (reduce_conservatively) ? apply_pass(import_file, &pass_replace_loads_individual, irg_ident) : apply_pass(import_file, &pass_replace_loads, irg_ident);
}