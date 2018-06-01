#include <libfirm/firm.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pass_utils.h>

void replace_selector(ir_node* node, ir_graph* irg) {
    srand(time(NULL));
    ir_tarval* val = rand() % 2 ? get_tarval_b_true() : get_tarval_b_false();
    dbg_info * dbgi = get_irn_dbg_info(node);
    ir_node* const_node = new_rd_Const(dbgi, irg, val);

    set_Switch_selector(node, const_node);
}

int pass_replace_switch_selectors_individual(ir_graph* irg, void* data) {
    
    ir_node_container* container = malloc(sizeof(ir_node_container));
    container->func = is_Switch;
    collect_nodes(irg, container);

    if (container->nodes_n == 0) {
        free(container);
        return 0;
    }
    ir_node* node = container->nodes[rand() % container->nodes_n];
    replace_selector(node, irg);
    
    free(container);
    return 1;

}

int pass_replace_switch_selectors(ir_graph* irg, void* data) {
    
    ir_node_container* container = malloc(sizeof(ir_node_container));
    container->func = is_Switch;
    collect_nodes(irg, container);

    if (container->nodes_n == 0) {
        free(container);
        return 0;
    }
    for(int i = 0; i < container->nodes_n; i++) {
        replace_selector(container->nodes[i], irg);   
    }
    free(container);
    return 1;

}

int main(int argc, char** argv) {
    if(atoi(argv[1]) != -1) {
        return apply_pass(&pass_replace_switch_selectors, atoi(argv[1]));
    } else {
        return apply_pass(&pass_replace_switch_selectors_individual, -1);
    }
}