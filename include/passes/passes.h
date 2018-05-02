#ifndef PASSES_H
#define PASSES_H

#include <libfirm/firm.h>

int PASSES_N; // total number of passes
int PASSES_APPLIED; // number of applied passes

/**
 * type for pass functions
 */
typedef void pass_func(ir_graph* irg);

typedef struct pass_t {
    char* ident; // identifier for the pass
    pass_func* func; // pointer to the pass function
} pass_t;

typedef enum PASS_ID {
    DO_NOTHING,
    LIBFIRM_OPT,
} PASS_ID;

void apply_pass(ir_prog* irp);

void init_passes();

//passes
void do_nothing(ir_graph* irg);
void irg_apply_opt(ir_graph* irg);

#endif