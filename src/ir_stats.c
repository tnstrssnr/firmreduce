#include "ir_stats.h"
#include <stdlib.h>
#include <stdio.h>
#include "irgwalk.h"

void ir_stats_walker(ir_node* node, void* data) {

    ir_stats_t* curr_stats = (ir_stats_t*) data;
    curr_stats->node_n++; 
}

void print_stats(ir_stats_t* stats) {
    printf("\t# of nodes: %d\n", stats->node_n);
    printf("\t# of blocks: %d\n", stats->block_n);
    printf("\t# of irgs: %d\n", stats->irg_n);
    printf("\t# of types: %d\n\n", stats->type_n);
}

ir_stats_t* get_ir_stats(int ident) {

    ir_stats_t* stats = malloc(sizeof(ir_stats_t));

    if(stats == NULL) {
        perror("Error: ");
        exit(1);
    }

    stats->ident = ident;
    stats->block_n = 0;
    stats->mem_node_n = 0;
    stats->node_n = 0;

    all_irg_walk(NULL, ir_stats_walker, stats);
    stats->irg_n = get_irp_n_irgs();
    stats->type_n = get_irp_n_types();
    
    print_stats(stats);
    return stats;
}

ir_stats_t* compare_stats(ir_stats_t* old, ir_stats_t* new) {
    int diff_node_n = old->node_n - new->node_n;


    // TODO: find criteria, when to update current  variant --> find passes first, see what they do!
    if(diff_node_n > 0) {
        return new;
    }

    return old;
}