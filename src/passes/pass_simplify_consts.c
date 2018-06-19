#include <libfirm/firm.h>
#include <pass_utils.h>
#include <stdlib.h>
#include <time.h>

/*
 * Replace tarval of all Const nodes w/ 0
 */

int is_Const_non_null(const ir_node* node) {
    return (is_Const(node) && !(tarval_is_null(get_Const_tarval(node))));
}

void simplify_const(ir_node* node) {
    ir_tarval* old = get_Const_tarval(node);
    const ir_mode* mode = get_tarval_mode(old);

    set_Const_tarval(node, get_mode_null(mode));
}

int pass_simplify_consts_individual(ir_graph* irg, void* data) {
    ir_node_container* container = new_container(is_Const_non_null);
    collect_nodes(irg, container);

    if(container->nodes_n == 0) return 0;

    srand(time(NULL));
    ir_node* node = container->nodes[rand() % container->nodes_n];
    simplify_const(node);
    free(container);
    return 1;
}

int pass_simplify_consts(ir_graph* irg, void* data) {
    ir_node_container* container = new_container(is_Const_non_null);
    collect_nodes(irg, container);

    if(container->nodes_n == 0) return 0;
    int* random_order = get_shuffle(container->nodes_n);

    for(int i = 0; i < container->nodes_n; i++) {
        ir_node* node = container->nodes[random_order[i]];
        simplify_const(node);
    }
    free(container);
    free(random_order);
    return 1;
}

int main(int argc, char** argv) {
    if(atoi(argv[3]) == 1) {
        return apply_pass(argv[1], &pass_simplify_consts_individual, -1, argv[4]);
    } else {
        return apply_pass(argv[1], &pass_simplify_consts, atoi(argv[2]), NULL);
    }
}