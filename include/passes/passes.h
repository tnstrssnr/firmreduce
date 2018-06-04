#ifndef PASSES_H
#define PASSES_H

#include <ir_stats_structs.h>

int PASSES_N; // total number of passes
int PASSES_APPLIED; // number of applied passes

typedef struct pass_t {
    char* ident;
    char* path;
    } pass_t;

pass_t** passes; // array containing all available passes
void init_passes_dynamic();
int apply_pass(char* path, int i, int individual, int reduce_individual);

#endif