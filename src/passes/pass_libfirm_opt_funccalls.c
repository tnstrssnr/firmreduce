#include <stdlib.h>
#include <pass_utils.h>

int main(int argc, char* argv[]) {
    if(argc != 6) {
        fprintf(stderr, "Unexpected number of arguments on call %s\n", argv[0]);
        exit(-1);
    }

    char* file = argv[1];
    char* dump = argv[2];
    int reduce_conservatively = atoi(argv[3]);
    char* ident = argv[4];

    if(reduce_conservatively) return 0; // libfirm optimization are all or nothing

    ir_init();
    if(ir_import(file)) {
        fprintf(stderr, "Error while reading test-case file\n");
        return -1;
    }

    ir_graph* irg = get_irg_by_ident(ident);
    if(!irg) {
        ir_finish();
        return 0;
    } // we may have already removed the irg

    // get node count before the optimization
    ir_node_container* container = new_container(select_all);
    collect_nodes(irg, container);
    int nodes_n_old = container->nodes_n;

    optimize_funccalls();

    // check if number of nodes changed
    // If not we assume that the optimization was ineffective and return 0
    collect_nodes(irg, container);
    int improvement = (container->nodes_n < nodes_n_old) ? 1 : 0;

    if(is_valid()) {
        ir_export(dump);
    } else {
        improvement = -1;
    }
    ir_finish();
    return improvement;

}