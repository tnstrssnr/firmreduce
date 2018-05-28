#include <stdlib.h>
#include <string.h>

#include <pass_utils.h>

const char* TEMP_DIR = "temp/";
const char* TEMP_IR = "temp/temp.ir";
const char* CURR_IR = "temp/curr.ir";

void find_nodes_walk(ir_node* node, void* data) {

    ir_node_container* container = (ir_node_container*) data;

    if((sizeof(container->nodes)) <= container->nodes_n) {
        ir_node** cond_nodes_new = malloc(2*sizeof(container->nodes));
        memcpy(cond_nodes_new, container->nodes, sizeof(ir_node*)*container->nodes_n);
        free(container->nodes);
        container->nodes = cond_nodes_new;
    }

    if((container->func)(node)) {
        container->nodes[container->nodes_n] = node;
        container->nodes_n++;
    }

}

void collect_nodes(ir_graph* irg, ir_node_container* container) {

    container->nodes = malloc(10*sizeof(ir_node*));
    container->nodes_n = 0;
    irg_walk_graph(irg, find_nodes_walk, NULL, container);

}

ir_node_container* new_container(select_func* func) {
    ir_node_container* container = malloc(sizeof(ir_node_container));
    container->func = func;
    return container;
}

int  is_valid() {
    // see if variant is a valid irp
    for(int i = 0; i < get_irp_n_irgs(); i++) {   
        if(!irg_verify(get_irp_irg(i))) {
                return 0;
        }
    }
    return 1;
}

/**
 * Convenience function for creating new passes
 * Pass needs a pass_func, that does something to the irg, the rest
 * is handled in this function
 * 
 * (assumes that pass_func doesn't need additional data)
 */
int apply_pass(pass_func* func) {
    // init library load current irp
    ir_init();
    if(ir_import(CURR_IR)) {
        fprintf(stderr, "Error while reading test-case file\n");
        return -1;
    }

    // apply pass
    int improvement = 0;
    for(int i = 0; i < get_irp_n_irgs(); i++) {
        ir_graph* irg = get_irp_irg(i);
        int result = (func)(irg, NULL);
        improvement = (improvement) ? 1 : result;

        // apply optimizations
        opt_bool(irg);
        optimize_cf(irg);
        opt_if_conv(irg);
        do_loop_inversion(irg);
        optimize_reassociation(irg);
        optimize_load_store(irg);
        optimize_graph_df(irg);
        combo(irg);
        scalar_replacement_opt(irg);
        place_code(irg);
        optimize_reassociation(irg);
        optimize_graph_df(irg);
        opt_jumpthreading(irg);
        optimize_graph_df(irg);
        construct_confirms(irg);
        optimize_graph_df(irg);
        remove_confirms(irg);
        optimize_cf(irg);
        optimize_load_store(irg);
        optimize_graph_df(irg);
        combo(irg);
        place_code(irg);
    }

    // check if we still have a valid irp
    if(is_valid()) {
        ir_export(TEMP_IR);
    }
    ir_finish();
    return (improvement) ? 1 : 0;

}