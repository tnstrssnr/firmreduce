#include <firm.h>
#include <pass_utils.h>

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