#include <stdio.h>
#include <stdlib.h>

#include "passes/passes.h"

pass_t** passes;

pass_t* new_pass(char* ident, pass_func* func) {

    pass_t* new_pass = malloc(sizeof(pass_t));
    new_pass->ident = ident;
    new_pass->func = func;
    return new_pass;

}

void init_passes() {
    PASSES_N = 2;
    PASSES_APPLIED = 0;
    passes = malloc(sizeof(pass_t) * PASSES_N);
    
    passes[DO_NOTHING] = new_pass("DO NOTHING", &do_nothing);
    passes[LIBFIRM_OPT] = new_pass("LIBFIRM OPT", &irg_apply_opt);
}

void apply_pass(ir_prog* irp) {

    pass_t* pass = passes[PASSES_APPLIED % PASSES_N];

    if (pass == NULL) {
        //pass not found -- what now?
        printf("Pass not found. Skipping.");
        return;
    }

    printf("Applying pass : %s\n", pass->ident);
    for(int i = 0; i < get_irp_n_irgs(); i++) {
        pass->func(get_irp_irg(i));
    }

    PASSES_APPLIED++;

}