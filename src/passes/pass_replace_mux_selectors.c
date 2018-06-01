#include <libfirm/firm.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pass_utils.h>

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
    
    free(container);
    return 1;

}

int pass_replace_mux_selectors(ir_graph* irg, void* data) {
    
    ir_node_container* container = malloc(sizeof(ir_node_container));
    container->func = is_Mux;
    collect_nodes(irg, container);

    if (container->nodes_n == 0) {
        free(container);
        return 0;
    }
    for(int i = 0; i < container->nodes_n; i++) {
        ir_node* node = container->nodes[i];
        srand(time(NULL));
        ir_tarval* val = rand() % 2 ? get_tarval_b_true() : get_tarval_b_false();
        dbg_info * dbgi = get_irn_dbg_info(node);
        ir_node* const_node = new_rd_Const(dbgi, irg, val);

        set_Mux_sel(node, const_node);
    }
    free(container);
    return 1;

}

int main(int argc, char** argv) {
    if(atoi(argv[1]) != -1) {
        return apply_pass(&pass_replace_mux_selectors, atoi(argv[1]));
    } else {
        return apply_pass(&pass_replace_mux_selectors_individual, -1);
    }
}