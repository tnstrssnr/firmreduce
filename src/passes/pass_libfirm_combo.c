#include <stdlib.h>
#include <pass_utils.h>

int main(int argc, char* argv[]) {
    char* file = argv[1];
    int irg = atoi(argv[2]);

    return apply_optimization(file, irg, combo);

}