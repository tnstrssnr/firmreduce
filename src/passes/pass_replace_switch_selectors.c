#include <libfirm/firm.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pass_utils.h>

/*
 * Replace all switch selectors w/ Const true or false
 */

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
    
    collect_nodes(irg, container);
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

    int* random_order = get_shuffle(container->nodes_n);
    for(int i = 0; i < container->nodes_n; i++) {
        replace_selector(container->nodes[random_order[i]], irg);   
    }
    free(container);
    free(random_order);
    return 1;

}

int main(int argc, char** argv) {
    if(argc != 5) {
        fprintf(stderr, "Unexpected number of arguments on call %s\n", argv[0]);
        exit(-1);
    }

    char* import_file = argv[1];
    char* dump = argv[2];
    int reduce_conservatively = atoi(argv[3]);
    char* irg_ident = argv[4];
    
    return (reduce_conservatively) ? apply_pass(import_file, dump, &pass_replace_switch_selectors_individual, irg_ident) : apply_pass(import_file, dump, &pass_replace_switch_selectors, irg_ident);
}