#include <libfirm/firm.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pass_utils.h>

/*
 * Replace Cond selectors w/ Const true or false
 */

int pass_replace_cond_selectors_individual(ir_graph* irg, void* data) {
    
    ir_node_container* container = malloc(sizeof(ir_node_container));
    container->func = is_Cond;
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

    set_Cond_selector(node, const_node);
    
    free(container);
    return 1;

}

int pass_replace_cond_selectors(ir_graph* irg, void* data) {
    
    ir_node_container* container = malloc(sizeof(ir_node_container));
    container->func = is_Cond;
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

        set_Cond_selector(node, const_node);
    }
    free(container);
    return 1;

}

int main(int argc, char** argv) {
    if(argc != 4) {
        return -1;
    }
    return (atoi(argv[3]) == 1) ? apply_pass(argv[1], &pass_replace_cond_selectors_individual, atoi(argv[2])) : apply_pass(argv[1], &pass_replace_cond_selectors, atoi(argv[2]));
}

