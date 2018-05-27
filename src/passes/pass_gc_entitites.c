#include <pass_utils.h>
#include <firm.h>

void gc_entities(ir_graph* irg, void* data) {
    // needs to be executed only once per irp, not once per irg
    static int done;
    if(!done) {
        garbage_collect_entities();
        done = 1;
    }
}

int main(int argc, char** argv) {
    return apply_pass(gc_entities);
}