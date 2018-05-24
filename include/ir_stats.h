#ifndef IR_STATS_H
#define IR_STATS_H

#include "libfirm/firm.h"

typedef struct ir_stats_t {
    int node_n; // number of nodes
    int mem_node_n; // number of mem-type nodes
    int cf_manips; //number of control flow manipulations
    int type_n; // number of types in the irp
    int irg_n; //number of irgs in the irp
} ir_stats_t;

typedef ir_stats_t* stats_func(char* path_to_file);

void print_stats(ir_stats_t* stats);

ir_stats_t* compare_stats(ir_stats_t* old, ir_stats_t* new);

#endif