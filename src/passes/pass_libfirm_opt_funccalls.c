#include <stdlib.h>
#include <pass_utils.h>

int main(int argc, char* argv[]) {
    char* file = argv[1];
    int irg = atoi(argv[2]);

    ir_init();
    if(ir_import(file)) {
        fprintf(stderr, "Error while reading test-case file\n");
        return -1;
    }

    // get node count before the optimization
    ir_node_container* container = new_container(select_all);
    collect_nodes(get_irp_irg(irg), container);
    int nodes_n_old = container->nodes_n;

    optimize_funccalls();

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