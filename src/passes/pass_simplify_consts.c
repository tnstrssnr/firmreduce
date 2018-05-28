#include <libfirm/firm.h>
#include <pass_utils.h>
#include <stdlib.h>
#include <time.h>

int is_Const_non_null(const ir_node* node) {
    return (is_Const(node) && !(tarval_is_null(get_Const_tarval(node))));
}

int pass_simplify_consts(ir_graph* irg, void* data) {
    ir_node_container* container = new_container(is_Const_non_null);
    collect_nodes(irg, container);

    if(container->nodes_n == 0) return 0;

    srand(time(NULL));
    ir_node* node = container->nodes[rand() % container->nodes_n];
    ir_tarval* old = get_Const_tarval(node);
    const ir_mode* mode = get_tarval_mode(old);

    set_Const_tarval(node, get_mode_null(mode));

    free(container);
    return 1;
}

int main(int argc, char** argv) {
    return apply_pass(&pass_simplify_consts);
}