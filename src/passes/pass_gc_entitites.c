#include <pass_utils.h>
#include <firm.h>
#include <stdlib.h>

int gc_entities(ir_graph* irg, void* data) {
    // needs to be executed only once per irp, not once per irg
    static int done = 0;
    if(!done) {
        garbage_collect_entities();
        done = 1;
    }
    return 0;
}

int main(int argc, char** argv) {
    return apply_pass(argv[1], gc_entities, atoi(argv[2]));
}