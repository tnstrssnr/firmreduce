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

char* LOG_FILE;

void log_text(char* text) {
    FILE* f = fopen(LOG_FILE, "a");
    fprintf(f, "%s", text);
    fclose(f);
}

void log_stats(ir_stats_t* stats) {
    char text[128]; // TODO: make sure buff size is big enough
    sprintf(text, "\t# of nodes: \t\t\t%d\n\t# of irgs: \t\t\t\t%d\n\t# of cf manipulations: \t%d\n\t# of memory operations: %d\n\t# of types: \t\t\t%d\n\n%c",
         stats->node_n, stats->irg_n, stats->cf_manips, stats->mem_node_n, stats->type_n, '\0');
    log_text(text);
}

void log_result(int result) {
    switch(result) {
        case 0:
            log_text("No improvement");
            break;
        case 1:
            log_text("Successful");
            break;
        default:
            log_text("Failed");            
            break;
    }
}

void init_logging(char* out_path) {
    LOG_FILE = malloc(sizeof(out_path) + 28);
    sprintf(LOG_FILE, "%s/Reduction.log%c", out_path, '\0');
    FILE* f = fopen(LOG_FILE, "w");
    if (!f) {
        perror("fopen");
        exit(1);
    }

    char buff[128];
    struct tm* time_;
    time_t now = time(0);
    time_ = localtime(&now);

    strftime (buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", time_);
    fprintf (f, "%s -- ", buff);
    fprintf(f, "Firmreduce -- Results\n\nInitial Test-case size:\n");
    fclose(f);
}