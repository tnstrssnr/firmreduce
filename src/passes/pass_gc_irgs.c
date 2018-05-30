#include <pass_utils.h>
#include <firm.h>

int gc_irgs_(ir_graph* irg, void* data) {
    // needs to be executed only once per irp, not once per irg
    static int done = 0;
    int irgs = get_irp_n_irgs();
    if(!done) {
        ir_entity* main_ent = get_irg_entity(get_irp_main_irg());
        ir_entity* keep_arr[1];
        keep_arr[0] = main_ent;
        gc_irgs(1, keep_arr);
        done = 1;
    }
    return (irgs > get_irp_n_irgs());
    
}

int main(int argc, char** argv) {
    //return apply_pass(gc_irgs_, -1);
    return 0;
}