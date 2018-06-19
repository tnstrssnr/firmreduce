#include <pass_utils.h>
#include <firm.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char** argv) {

    if (atoi(argv[3]) == -1) return 0; // we can't so conservative reduction here

    ir_init();
    if(ir_import(argv[1])) {
        fprintf(stderr, "Error while reading test-case file\n");
        return -1;
    }
    garbage_collect_entities();

    // check if we still have a valid irp
    if(is_valid()) {
        ir_export("temp/temp.ir");
    } else {
        ir_finish();
        return -1;
    }
    ir_finish();

    int input_size;
    int output_size;

    struct stat st;
    if(stat("temp/temp.ir", &st) == 0) {
        output_size = st.st_size;
    }
    if(stat("temp/curr.ir", &st) == 0) {
        input_size = st.st_size;
    }
    if(!input_size|| !output_size) {
        return -1;
    }

    // check to see if output file has gotten smaller

    return input_size > output_size;
}
    