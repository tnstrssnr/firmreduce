#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>
#include <dlfcn.h>
#include <assert.h>

#include "ir_stats_structs.h"
#include "passes/passes.h"
#include "logging.h"

char* IS_REPRODUCER_SCRIPT;
char* OUT_PATH = NULL;

void init_reproducer_test(const char* path, const char* reprod_args) {

    //first check if file path is valid
    struct stat st;
    if(stat(path, &st) < 0 || !((st.st_mode & S_IFMT) == S_IFREG)) {
        fprintf(stderr, "Error while reading %s: ", path);
        perror("");
        exit(1);
    }

    // We need 4 more characters
    IS_REPRODUCER_SCRIPT = malloc(strlen(path) + strlen(reprod_args) + 4);

    if (IS_REPRODUCER_SCRIPT == NULL) {
        //malloc failed, deal with it
        perror("Error");
        exit(1);
    }
    sprintf(IS_REPRODUCER_SCRIPT, "./%s %s%c", path, reprod_args, '\0');
}

void init_temp_dirs(char* ir_path) {

    if(OUT_PATH == NULL) {
        //path is valid, we already checked
        OUT_PATH = malloc(sizeof(dirname(ir_path)));
        OUT_PATH = dirname(ir_path);
    } else {
        //check if path is valid
        struct stat st;
        if(stat(OUT_PATH, &st) < 0 || !((st.st_mode & S_IFMT) == S_IFDIR)) {
            fprintf(stderr, "Error while reading %s: ", OUT_PATH);
            perror("");
        }
    }
 
    /*
    // add '/' to path if necessary
    if(OUT_PATH[strlen(OUT_PATH)] != '/') {
        char out_path_new[strlen(OUT_PATH) + 1];
        sprintf(out_path_new,"%s%c%c", OUT_PATH, '/', '\0');
        OUT_PATH = out_path_new;
    }
    */
    
    // move and rename initial test-case to temp directory    
    system("mkdir -p temp");
    char cp_cmd[strlen(ir_path) + 20];
    sprintf(cp_cmd, "cp %s temp/curr.ir%c", ir_path, '\0');
    system(cp_cmd);
}

ir_stats_t* get_ir_stats(char* path_to_file) {
    char* path_to_libstats = "build/debug/libstats";
    ir_stats_t* stats = malloc(sizeof(ir_stats_t));

    char command[strlen(path_to_libstats) + strlen(path_to_file) + strlen(OUT_PATH) + strlen("temp/stats") + 5];
    sprintf(command, "%s %s %s %s%c", path_to_libstats, path_to_file, OUT_PATH, "temp/stats", '\0');

    system(command);

    FILE* f = fopen("temp/stats", "r");
    assert(f != NULL);
    char* line = NULL;
    size_t size = 32;
    getline(&line, &size, f);

    char* node_n = strtok(line, " ");
    char* mem_node_n = strtok(NULL, " ");
    char* cf_manips= strtok(NULL, " ");
    char* type_n = strtok(NULL, " ");
    char* irg_n = strtok(NULL, " ");

    stats->node_n = atoi(node_n);
    stats->mem_node_n = atoi(mem_node_n);
    stats->cf_manips = atoi(cf_manips);
    stats->type_n = atoi(type_n);
    stats->irg_n = atoi(irg_n);
    return stats;

}

void init(char* rep_path, char* reprod_args, char* ir_path) {
    init_reproducer_test(rep_path, reprod_args);
    init_temp_dirs(ir_path);
    init_logging(OUT_PATH);
    init_passes_dynamic();
    log_stats(get_ir_stats("temp/curr.ir"));
}


void finish(ir_stats_t* final) {
    //TODO: print final statistics     
}

//execute shell script
int is_reproducer() {
    FILE* f = popen(IS_REPRODUCER_SCRIPT, "r");
    if(!f) {
        printf("Couldn't execute reproducer script.\n");
        exit(1);
    }
    char result = fgetc(f);
    fclose(f);
    return result;
}

/**
 * function handles all interaction w/ libstats object
 */
int has_improved() {

    char* replace = "mv temp/temp.ir temp/curr.ir";

    ir_stats_t* old = get_ir_stats("temp/curr.ir");
    ir_stats_t* new = get_ir_stats("temp/temp.ir");

    int changed = (old->node_n != new->node_n
               || old->mem_node_n != new->mem_node_n 
               || old->cf_manips != new->cf_manips
               || old->type_n != new->type_n
               || old->type_n != new->type_n) ? 1 : 0;
    
    if (changed) {
        log_text("Improved variant found:\n");
        log_stats(new);
        system(replace);
    }
    return changed;
}

void reduce() {

    /**
     * Greedy search of new variants:
     * loop through passes, check if we get a 'better' variant of the irp
     *     if yes: save as current variant
     *     if no: discard
     * repeat until full iteraton w/o improvement
     */

    int no_improvement = 0;
    int pass_failed = 0;
    int fixpoint = 0;
    int next_pass = 0;

    while(!fixpoint) {
        int result = apply_pass(next_pass);
        next_pass = (next_pass + 1) % PASSES_N;
        printf(".");
        if(result == -1)  pass_failed++;
        if(!has_improved() || !is_reproducer()) no_improvement++;

        if(no_improvement >= PASSES_N || pass_failed >= PASSES_N) {
            fixpoint = 1;
        }
    }
    printf("\n");
}

int main(int argc, char** argv) {
/*
 * Program expects 2 arguments:
 *     1. path to .sh file -> reproducer test for bug
 *     2. path to .ir file of input test-case
 * 
 * optional arguments:
 *     -o path/to/dir: Expects path to directory, where temp files and reduced test-cases are dumped
 *      If no path is specified, path of input test-case will be used
 * 
 *      -a "args": Arguments that should be passed to reproducer script
 */

    char* reprod_args = "";

    int i;
    while((i = getopt(argc, argv, "o:a:")) != -1) {
        switch(i) {
            case 'o':
                OUT_PATH = optarg;
                break;
            case 'a':
                reprod_args = optarg;
                break;
            case '?':
            default:
                fprintf(stderr, "Error while parsing arguments\n");
                exit(1);
        }
    }
    if (reprod_args == NULL) reprod_args = "";

    if(argv[optind] == NULL || argv[optind + 1] == NULL || argv[optind + 2] != NULL) {
        fprintf(stderr, "Received unexpected number of arguments\n");
        exit(1);
    } else {
        init(argv[optind], reprod_args, argv[optind + 1]);
    }
    reduce();
}