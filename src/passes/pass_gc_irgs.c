#include <pass_utils.h>
#include <firm.h>
#include <string.h>
#include <stdlib.h>

const char* MAIN = "main";

int main(int argc, char** argv) {
    char* import_file = argv[1];
    int irg_nr = atoi(argv[2]);
    int reduce_conservatively = atoi(argv[3]);

    int result = 0;
    if (reduce_conservatively) return 0;

    ir_init();

    if(ir_import(import_file)) {
        fprintf(stderr, "Error while reading test-case file\n");
        return -1;
    }

    int irg_n_old = get_irp_n_irgs();

    if(irg_nr >= irg_n_old) return 0;
    ir_entity* keep_arr[2];

    keep_arr[1] = get_irg_entity(get_irp_irg(irg_nr));

    for(int i = 0; i < irg_n_old; i++) {
        const char* pass_name = get_id_str(get_entity_ident(get_irg_entity(get_irp_irg(i))));
        if(strcmp(pass_name, MAIN) == 0) {
            keep_arr[0] = get_irg_entity(get_irp_irg(i));
            break;
        }
    }
    gc_irgs(2, keep_arr);
    int irg_n_new = get_irp_n_irgs();
    result = (irg_n_new < irg_n_old) ? 1 : 0;
    
    if(is_valid()) {
        ir_export("temp/temp.ir");
    } else {
        result = -1;
    }
    ir_finish();

    return result;
}