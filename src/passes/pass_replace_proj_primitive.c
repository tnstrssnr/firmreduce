#include <libfirm/firm.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pass_utils.h>

int is_proj_no_memop(const ir_node* node) {
    if (!is_Proj(node)) return 0;
    if (is_memop(node)) return 0;
    ir_mode* mode = get_irn_mode(node);
    if( mode_is_data(mode)) {
        return 1;
    }
    return 0;
}

/*
 * Replace Load nodes w/ Const values
 */
void replace_proj(ir_graph* irg, ir_node* node) {
    ir_mode* mode = get_irn_mode(node);
    ir_tarval* val = get_mode_one(mode);
    dbg_info * dbgi = get_irn_dbg_info(node);
    ir_node* const_node = new_rd_Const(dbgi, irg, val);

    edges_activate(irg);
    exchange(node, const_node);
    edges_deactivate(irg);
}

int pass_replace_proj_individual(ir_graph* irg, void* data) {
    
    ir_node_container* container = malloc(sizeof(ir_node_container));
    container->func = is_proj_no_memop;
    collect_nodes(irg, container);

    if (container->nodes_n == 0) {
        free(container);
        return 0;
    }
    srand(time(NULL));
    ir_node* node = container->nodes[rand() % container->nodes_n];
    replace_proj(irg, node);

    free(container);
    return 1;

}

int pass_replace_proj(ir_graph* irg, void* data) {
    
    ir_node_container* container = malloc(sizeof(ir_node_container));
    container->func = is_proj_no_memop;
    collect_nodes(irg, container);

    if (container->nodes_n == 0) {
        free(container);
        return 0;
    }
    int* random_order = get_shuffle(container->nodes_n);

    for(int i = 0; i < container->nodes_n; i++) {
        ir_node* node = container->nodes[random_order[i]];
        replace_proj(irg, node);
    }

    free(container);
    free(random_order);
    return 1;

}

int main(int argc, char** argv) {
    char* import_file = argv[1];
    int reduce_conservatively = atoi(argv[2]);
    char* irg_ident = argv[3];

    return (reduce_conservatively) ? apply_pass(import_file, &pass_replace_proj_individual, irg_ident) : apply_pass(import_file, &pass_replace_proj, irg_ident);
}