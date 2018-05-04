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

void apply_pass(ir_prog* irp);

void init_passes();

void init_passes_dynamic();

#endif