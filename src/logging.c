#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>
#include <dlfcn.h>
#include <time.h>

#include "logging.h"

const char* LOG_FILE;

void log_text(char* text) {
    FILE* f = fopen(LOG_FILE, "a");
    fprintf(f, "%s", text);
    fclose(f);
}

void log_stats(ir_stats_t* stats) {
    char text[128];
    sprintf(text, "\t# of nodes: \t\t\t%d\n\t# of irgs: \t\t\t\t%d\n\t# of cf manipulations: \t%d\n\t# of memory operations: %d\n\t# of types: \t\t\t%d\n\n%c",
         stats->node_n, stats->irg_n, stats->cf_manips, stats->mem_node_n, stats->type_n, '\0');
    log_text(text);
    
}

void init_logging(char* out_path) {
    
    int i = 0;
    char* file_path = NULL;

    // increase counter on Reductionx.log, until we find filename, that is not taken yet
    do {
        if(file_path != NULL) free(file_path);
        file_path = malloc(sizeof(out_path) + 15 + (i / 10));
        if (i > 0) {
            sprintf(file_path, "%sReduction%d.log%c", out_path, i , '\0');
        } else {
            sprintf(file_path, "%sReduction.log%c", out_path, '\0');
        }
        i++;
    } while (access(file_path, F_OK) == 0);
    
    
    LOG_FILE = file_path;

    FILE* log_file;
    log_file = fopen(file_path, "a+");
    if(log_file == NULL) {
        fprintf(stderr, "Couldn't create log-file");
        exit(1);
    }

    char buff[20];
    struct tm* time_;
    time_t now = time(0);
    time_ = gmtime(&now);

    strftime (buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", time_);
    fprintf (log_file, "%s -- ", buff);
    fprintf(log_file, "Firmreduce -- Results\n\nInitial Test-case size:\n");
    fclose(log_file);
}