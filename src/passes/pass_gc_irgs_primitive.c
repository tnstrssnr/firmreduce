#include <pass_utils.h>
#include <firm.h>
#include <string.h>
#include <stdlib.h>

const char* MAIN = "main";

int main(int argc, char** argv) {
    if(argc != 5) {
        fprintf(stderr, "Unexpected number of arguments on call %s\n", argv[0]);
        exit(-1);
    }

    char* import_file = argv[1];
    char* dump = argv[2];
    int reduce_conservatively = atoi(argv[3]);
    char* irg_ident = argv[4];

    if (reduce_conservatively) return 0;
    
    int result = 0;

    ir_init();

    if(ir_import(import_file)) {
        fprintf(stderr, "Error while reading test-case file\n");
        return -1;
    }

    int irg_n_old = get_irp_n_irgs();

    ir_graph* irg = get_irg_by_ident(irg_ident);
    if(!irg) return 0; // we may have already removed the irg;

    const char* irg_name = get_id_str(get_entity_ident(get_irg_entity(irg)));
    if(strcmp(irg_name, MAIN) == 0) return 0;

    free_ir_graph(irg);
    
    int irg_n_new = get_irp_n_irgs();
    result = (irg_n_new < irg_n_old) || 0;
    
    if(is_valid()) {
        ir_export(dump);
    } else {
        result = -1;
    }
    ir_finish();

    return result;
}