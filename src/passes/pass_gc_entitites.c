#include <pass_utils.h>
#include <firm.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char** argv) {
    if(argc != 6) {
        fprintf(stderr, "Unexpected number of arguments on call %s\n", argv[0]);
        exit(-1);
    }

    char* import_file = argv[1];
    char* dump = argv[2];
    int reduce_conservatively = atoi(argv[3]);

    if (reduce_conservatively == 1) return 0; // we can't so conservative reduction here

    ir_init();
    if(ir_import(import_file)) {
        fprintf(stderr, "Error while reading test-case file\n");
        return -1;
    }
    garbage_collect_entities();

    // check if we still have a valid irp
    if(is_valid()) {
        ir_export(dump);
    } else {
        ir_finish();
        return -1;
    }
    ir_finish();

    int input_size;
    int output_size;

    struct stat st;
    if(stat(dump, &st) == 0) {
        output_size = st.st_size;
    }
    if(stat(import_file, &st) == 0) {
        input_size = st.st_size;
    }
    if(!input_size || !output_size) {
        return -1;
    }

    // check to see if output file has gotten smaller

    return input_size > output_size;
}
    