#include <firm.h>
#include <pass_utils.h>

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

int is_arithmetic(const ir_node* node) {
    return (is_Add(node) || is_Mul(node) || is_Mulh(node) || is_Div(node) || is_Sub(node));
}

int pass_replace_arithmetic_individual(ir_graph* irg, void* data) {

    ir_node_container* container = new_container(is_arithmetic);
    collect_nodes(irg, container);

    if(container->nodes_n == 0) return 0;

    srand(time(NULL));
    ir_node* node = container->nodes[rand() % container->nodes_n];

    ir_mode* mode = get_irn_mode(node);
    ir_tarval* val = get_mode_null(mode);
    dbg_info * dbgi = get_irn_dbg_info(node);
    ir_node* const_node = new_rd_Const(dbgi, irg, val);

    edges_activate(irg);
    exchange(node, const_node);
    edges_deactivate(irg);
    free(container);
    return 1;

}

int pass_replace_arithmetic(ir_graph* irg, void* data) {

    ir_node_container* container = new_container(is_arithmetic);
    collect_nodes(irg, container);

    if(container->nodes_n == 0) return 0;

    edges_activate(irg);
    for(int i = 0; i < container->nodes_n; i++) {
        ir_node* node = container->nodes[i];

        ir_mode* mode = get_irn_mode(node);
        ir_tarval* val = get_mode_null(mode);
        dbg_info * dbgi = get_irn_dbg_info(node);
        ir_node* const_node = new_rd_Const(dbgi, irg, val);

        
        exchange(node, const_node);

    }
    edges_deactivate(irg);
    free(container);
    return 1;

}

int main(int argc, char** argv) {
    if (argc > 1) {
        return apply_pass(&pass_replace_arithmetic);
    } else {
        return apply_pass(&pass_replace_arithmetic_individual);
    }
}