#include <stdlib.h>
#include <pass_utils.h>

int main(int argc, char* argv[]) {
    if(argc != 5) {
        fprintf(stderr, "Unexpected number of arguments on call %s\n", argv[0]);
        exit(-1);
    }

    char* import_file = argv[1];
    char* dump = argv[2];
    int reduce_conservatively = atoi(argv[3]);
    char* irg_ident = argv[4];

    return apply_optimization(file, dump, irg_ident, construct_confirms);

}