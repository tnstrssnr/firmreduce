#include <libfirm/firm.h>
#include <stdlib.h>
#include <time.h>


/**
 * Pass substitutes all Cond selectors w/ Const nodes
 */
void substitute_bools_walk(ir_node* node, void* data) {

    if(is_Cond(node)) {
        int rand_num = rand() % 2;
        ir_tarval* val = rand_num ? get_tarval_b_true() : get_tarval_b_false();
        dbg_info * dbgi = get_irn_dbg_info(node);
        ir_graph* irg = get_irn_irg(node);
        ir_node* const_node = new_rd_Const(dbgi, irg, val);
        
        set_Cond_selector(node, const_node);
    }

}

void pass_replace_cond_selectors(ir_graph* irg, void* data) {
    srand(time(NULL));
    irg_walk_graph(irg, substitute_bools_walk, NULL, NULL);
    

}