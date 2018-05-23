#include <libfirm/firm.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pass_utils.h>

void pass_replace_cond_selectors(ir_graph* irg, void* data) {
    
    ir_node_container* container = malloc(sizeof(ir_node_container));
    container->func = is_Cond;
    collect_nodes(irg, container);

    if (container->nodes_n > 0) {

        srand(time(NULL));
        ir_node* node = container->nodes[rand() % container->nodes_n];
        ir_tarval* val = rand() % 2 ? get_tarval_b_true() : get_tarval_b_false();
        dbg_info * dbgi = get_irn_dbg_info(node);
        ir_node* const_node = new_rd_Const(dbgi, irg, val);
    
        set_Cond_selector(node, const_node);
    }

    free(container);

}

int main(int argc, char** argv) {
    return apply_pass(&pass_replace_cond_selectors);
}

