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
void init_passes_dynamic(char* path);
int apply_pass(char* path, char* dump, int pass_idx, int reduce_individual, char* ident, int seed);

#endif