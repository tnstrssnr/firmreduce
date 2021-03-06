#include <firm.h>
#include <pass_utils.h>

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

int is_Return_not_reduced_yet(const ir_node* node_) {
    ir_node* node = (ir_node*) node_;
    if(!is_Return(node)) return 0;
    int reduced = 0;

    ir_node** res_arr = get_Return_res_arr(node);
    int res_n = get_Return_n_ress(node);
    for(int i = 0; i < res_n; i++) {
        
        if(!is_Const(res_arr[i]) && get_irn_node_nr(get_Return_mem(node)) != get_irn_node_nr(res_arr[i])) {
            reduced = 1;
        }
    }
    return reduced;
}

int pass_replace_return_values(ir_graph* irg, void* data) {

    ir_node_container* container = new_container(is_Return_not_reduced_yet);
    collect_nodes(irg, container);

    if(container->nodes_n == 0) return 0;

    ir_node* node = container->nodes[0]; // an irg has only 1 return node max

    ir_node** res_arr = get_Return_res_arr(node);
    int res_n = get_Return_n_ress(node);

    for(int i = 0; i < res_n; i++) {
        ir_mode* mode = get_irn_mode(res_arr[i]);
        ir_tarval* val = get_mode_null(mode);
        dbg_info * dbgi = get_irn_dbg_info(res_arr[i]);
        ir_node* const_node = new_rd_Const(dbgi, irg, val);

        set_Return_res(node, i, const_node);
    }
    return 1;
}

int main(int argc, char** argv) {
    if(argc != 6) {
        fprintf(stderr, "Unexpected number of arguments on call %s\n", argv[0]);
        exit(-1);
    }

    char* import_file = argv[1];
    char* dump = argv[2];
    int reduce_conservatively = atoi(argv[3]);
    char* irg_ident = argv[4];
    int seed = atoi(argv[5]);
    srand(seed);

    return (reduce_conservatively) ? apply_pass(import_file, dump, &pass_replace_return_values, irg_ident) : apply_pass(import_file, dump, &pass_replace_return_values, irg_ident);
}