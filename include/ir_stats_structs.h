#ifndef IR_STATS_STRUCTS_H
#define IR_STATS_STRUCTS_H

typedef struct ir_stats_t {
    int node_n; // number of nodes
    int mem_node_n; // number of mem-type nodes
    int cf_manips; //number of control flow manipulations
    int type_n; // number of types in the irp
    int irg_n; //number of irgs in the irp
} ir_stats_t;

#endif