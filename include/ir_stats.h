#ifndef IR_STATS_H
#define IR_STATS_H

#include "libfirm/firm.h"

typedef struct ir_stats_t {
    int ident; // identifier of testcase variant
    int node_n; // number of nodes
    int block_n; // number of blocks
    int mem_node_n; // number of mem-type nodes
    int type_n; // number of types in the irp
    int irg_n; //number of irgs in the irp
} ir_stats_t;

ir_stats_t* get_ir_stats(int ident);

void print_stats(ir_stats_t* stats);

ir_stats_t* compare_stats(ir_stats_t* old, ir_stats_t* new);

#endif