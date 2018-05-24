#include "ir_stats.h"
#include <stdlib.h>
#include <stdio.h>
#include "irgwalk.h"

void ir_stats_walker(ir_node* node, void* data) {

    ir_stats_t* curr_stats = (ir_stats_t*) data;
    curr_stats->node_n++; 
    if(is_cfop(node)) {
        curr_stats->cf_manips++;
    }
    if(is_memop(node)) {
        curr_stats->mem_node_n++;
    }
}

ir_stats_t* get_ir_stats(char* path_to_file) {

    ir_init();
    if(ir_import(path_to_file)) {
        fprintf(stderr, "Error while reading test-case file\n");
        exit(1);
    }

    ir_stats_t* stats = malloc(sizeof(ir_stats_t));

    if(stats == NULL) {
        perror("Error: ");
        exit(1);
    }

    stats->mem_node_n = 0;
    stats->node_n = 0;
    stats->cf_manips = 0;

    all_irg_walk(NULL, ir_stats_walker, stats);
    stats->irg_n = get_irp_n_irgs();
    stats->type_n = get_irp_n_types();

    ir_finish();
    
    return stats;
}