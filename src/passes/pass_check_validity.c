#include <firm.h>
#include <pass_utils.h>

/**
 * Checks validity of the input file
 * Returns 0 if file is ok, else -1
 */
int main(int argc, char** argv) {

    ir_init();
    
    if(ir_import(argv[1])) {
        fprintf(stderr, "Error while reading test-case file\n");
        return -1;
    }

    if(is_valid()) {
        ir_finish();
        return 0;
    }
    ir_finish();
    return -1;
}