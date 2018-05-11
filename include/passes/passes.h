#ifndef PASSES_H
#define PASSES_H

#include <libfirm/firm.h>

int PASSES_N; // total number of passes
int PASSES_APPLIED; // number of applied passes

/**
 * type for pass functions
 */
typedef void pass_func(ir_graph* irg, void* data);

typedef struct pass_t {
    char* ident; // identifier for the pass
    void* handle; // a handle to the shared object, where the pass is loaded from
    pass_func* func; // pointer to the pass function
} pass_t;

pass_t** passes; // array containing all available passes

void apply_pass(ir_prog* irp);

void init_passes_dynamic();

#endif