#include <stdlib.h>
#include <stdio.h>
#include "irgwalk.h"
#include <stdbool.h>

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

/**
 * Receives the following arguments:
 *  - path to .ir file, for which stats should be returned
 *  - path to directory, to dump the stats file in
 *  - 0/1, indicating whether .vcg files should be dumped
 *  - path to directory, to dump the graphs in
 *  - suffix for dumping graphs
 */
int main(int argc, char** argv) {

    if(argc != 6) {
        fprintf(stderr, "Unexpected number of arguments\n");
        exit(1);
    }

    char* import_file = argv[1];
    char* dump_stats = argv[2];
    bool dump = atoi(argv[3]);
    char* dump_graphs = argv[4];
    char* suffix = argv[5];

    ir_init();
    if(ir_import(import_file)) {
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

    // dump vcg files
    if(dump) {
        ir_set_dump_path(dump_graphs);
        for(int i = 0; i < get_irp_n_irgs(); i++) {
            dump_ir_graph(get_irp_irg(i), suffix);
        }
    }
    
    // write stats + irg identifiers to file
    FILE* f = fopen(dump_stats, "w+");
    fprintf(f, "%d %d %d %d %d\n", stats->node_n, stats->mem_node_n, stats->cf_manips, stats->type_n, stats->irg_n);
    for(int i = 0; i < get_irp_n_irgs(); i++) {
        fprintf(f, "%s ", get_id_str(get_entity_ident(get_irg_entity(get_irp_irg(i)))));
    }
    fprintf(f, "\n");
    fclose(f);
    ir_finish();

    return 0;
}