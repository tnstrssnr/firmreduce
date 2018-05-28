#include <pass_utils.h>
#include <firm.h>

int gc_irgs_(ir_graph* irg, void* data) {
    // needs to be executed only once per irp, not once per irg
    static int done = 0;
    if(!done) {
        ir_graph* main_irg = get_irp_irg(1);
        ir_entity* main_ent = get_irg_entity(main_irg);
        ir_entity* keep_arr[1];
        keep_arr[0] = main_ent;
        gc_irgs(1, keep_arr);
        done = 1;
    }
    return 0;
}

int main(int argc, char** argv) {
    //return apply_pass(gc_irgs_);
    return 0;
}