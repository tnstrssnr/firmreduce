#include <stdlib.h>
#include <pass_utils.h>

int main(int argc, char* argv[]) {
    char* file = argv[1];
    int reduce_conservatively = atoi(argv[2]);
    char* irg_ident = argv[3];

    return apply_optimization(file, irg_ident, normalize_n_returns);

}