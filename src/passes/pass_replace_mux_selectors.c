#include <libfirm/firm.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pass_utils.h>

/*
 * Replace all Mux selectors w/ Const true or false
 */
int pass_replace_mux_selectors_individual(ir_graph* irg, void* data) {
    
    ir_node_container* container = malloc(sizeof(ir_node_container));
    container->func = is_Mux;
    collect_nodes(irg, container);

    if (container->nodes_n == 0) {
        free(container);
        return 0;
    }
    srand(time(NULL));
    ir_node* node = container->nodes[rand() % container->nodes_n];
    ir_tarval* val = rand() % 2 ? get_tarval_b_true() : get_tarval_b_false();
    dbg_info * dbgi = get_irn_dbg_info(node);
    ir_node* const_node = new_rd_Const(dbgi, irg, val);

    set_Mux_sel(node, const_node);
    
    collect_nodes(irg, container);
    int left_to_do = container->nodes_n;
    free(container);
    return left_to_do;

}

int pass_replace_mux_selectors(ir_graph* irg, void* data) {
    
    ir_node_container* container = malloc(sizeof(ir_node_container));
    container->func = is_Mux;
    collect_nodes(irg, container);

    if (container->nodes_n == 0) {
        free(container);
        return 0;
    }

    int* random_order = get_shuffle(container->nodes_n);
    for(int i = 0; i < container->nodes_n; i++) {
        ir_node* node = container->nodes[random_order[i]];
        srand(time(NULL));
        ir_tarval* val = rand() % 2 ? get_tarval_b_true() : get_tarval_b_false();
        dbg_info * dbgi = get_irn_dbg_info(node);
        ir_node* const_node = new_rd_Const(dbgi, irg, val);

        set_Mux_sel(node, const_node);
    }
    collect_nodes(irg, container);
    int left_to_do = container->nodes_n;
    free(container);
    free(random_order);
    return left_to_do;

}

int main(int argc, char** argv) {
    return apply_pass(argv[1], &pass_replace_mux_selectors_individual, 0);
}