#include <stdlib.h>
#include <string.h>
#include <adt/array.h>
#include <time.h>

#include <pass_utils.h>

const char* TEMP_DIR = "temp/";
const char* TEMP_IR = "temp/temp.ir";
const char* CURR_IR = "temp/curr.ir";

void find_nodes_walk(ir_node* node, void* data) {

    ir_node_container* container = (ir_node_container*) data;

    if((container->func)(node)) {
        ARR_APP1(ir_node*, container->nodes, node);
        container->nodes_n++;
    }

}

void collect_nodes(ir_graph* irg, ir_node_container* container) {

    container->nodes = NEW_ARR_F(ir_node*, 0);
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
 * Uses the Fisher-Yates shuffle to create a random permutation of the array {0,...,size-1}
 * This is used to randomize the order in which passes are applied to the testcase
 */
int* get_shuffle(int size) {
    int* arr = malloc(size* sizeof(int));

    for(int i = 0; i < size; i++) {
        arr[i] = i;
    }

    srand(time(NULL));
    for(int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
    return arr;
}

/**
 * Convenience function for creating new passes
 * Pass needs a pass_func, that does something to the irg, the rest
 * is handled in this function
 * 
 * (assumes that pass_func doesn't need additional data)
 */
int apply_pass(char* path, pass_func* func, int idx, char* ident) {

    // init library, load current irp
    ir_init();
    if(ir_import(path)) {
        fprintf(stderr, "Error while reading test-case file\n");
        return -1;
    }

    ir_graph* irg;
    if(idx == -1) {
        irg = get_irg_by_ident(ident);
    } else {
        irg = get_irp_irg(idx);
    }
    int improvement = (func)(irg, NULL);

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
    
    // check if we still have a valid irp
    if(is_valid()) {
        ir_export("temp/temp.ir");
    } else {
        improvement = -1;
    }

    ir_finish();

    return improvement;
}

// always returns 1, bc we want all nodes to be selected
int select_all(const ir_node* node) {
    return 1;
}

int apply_optimization(char* file, int irg, opt_func* func) {
    if (irg == -1) return 0;
    ir_init();
    if(ir_import(file)) {
        fprintf(stderr, "Error while reading test-case file\n");
        return -1;
    }

    // get node count before the optimization
    ir_node_container* container = new_container(select_all);
    collect_nodes(get_irp_irg(irg), container);
    int nodes_n_old = container->nodes_n;

    (func)(get_irp_irg(irg));

    // check if number of nodes changed
    // If not we assume that the optimization was ineffective and return 0
    collect_nodes(get_irp_irg(irg), container);
    int improvement = (container->nodes_n < nodes_n_old) ? 1 : 0;

    if(is_valid()) {
        ir_export("temp/temp.ir");
    } else {
        improvement = -1;
    }
    ir_finish();
    return improvement;
}

/**
 * returns irg w/ entity identifier *ident*, NULL otherwise
 * allows pass to identify irg on which the aggressive reduction failed (ident is passed as argument)
 */
ir_graph* get_irg_by_ident(const char* ident) {
    ir_graph* irg;
    for(int i = 0; i < get_irp_n_irgs(); i++) {
        if(strcmp(ident, get_id_str(get_entity_ident(get_irg_entity(get_irp_irg(i))))) == 0) {
            irg =  get_irp_irg(i);
        } 
    }
    return irg;
}