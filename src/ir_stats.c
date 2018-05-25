#include <stdlib.h>
#include <stdio.h>
#include "irgwalk.h"

#include "libfirm/firm.h"
#include "ir_stats_structs.h"

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

int main(int argc, char** argv) {

    if(argc != 4) {
        fprintf(stderr, "Unexpected number of arguments\n");
        exit(1);
    }

    ir_init();
    if(ir_import(argv[1])) {
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

    ir_set_dump_path(argv[2]);
    dump_all_ir_graphs("");

    ir_finish();
    
    FILE* f = fopen(argv[3], "w+");
    fprintf(f, "%d %d %d %d %d\n", stats->node_n, stats->mem_node_n, stats->cf_manips, stats->type_n, stats->irg_n);
    fclose(f);

    return 0;
}