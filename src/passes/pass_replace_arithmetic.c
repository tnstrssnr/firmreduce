#include <firm.h>
#include <pass_utils.h>

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

/*
 * Remove arithmetic nodes, such as Add, Sub, Mul,... w/ Const value 0
 */

int is_arithmetic(const ir_node* node) {
    return (is_Add(node) || is_Mul(node) || is_Mulh(node) || is_Div(node) || is_Sub(node)
    || is_Shl(node) || is_Shr(node) || is_Shrs(node) || is_Mod(node));
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
    collect_nodes(irg, container);
    int left_to_do = container->nodes_n;
    free(container);
    return left_to_do;

}

int pass_replace_arithmetic(ir_graph* irg, void* data) {

    ir_node_container* container = new_container(is_arithmetic);
    collect_nodes(irg, container);

    if(container->nodes_n == 0) return 0;

    int* random_order = get_shuffle(container->nodes_n);
    edges_activate(irg);
    for(int i = 0; i < container->nodes_n; i++) {
        ir_node* node = container->nodes[random_order[i]];

        ir_mode* mode = get_irn_mode(node);
        ir_tarval* val = get_mode_null(mode);
        dbg_info * dbgi = get_irn_dbg_info(node);
        ir_node* const_node = new_rd_Const(dbgi, irg, val);
        
        exchange(node, const_node);

    }
    edges_deactivate(irg);
    collect_nodes(irg, container);
    int left_to_do = container->nodes_n;
    free(container);
    free(random_order);
    return left_to_do;

}

int main(int argc, char** argv) {
    if(argc != 4) {
        return -1;
    }
    return (atoi(argv[3]) == 1) ? apply_pass(argv[1], &pass_replace_arithmetic_individual, atoi(argv[2])) : apply_pass(argv[1], &pass_replace_arithmetic, atoi(argv[2]));
}