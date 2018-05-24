#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>
#include <dlfcn.h>

#include "ir_stats.h"
#include "passes/passes.h"

const char* IS_REPRODUCER;
char* OUT_PATH;
const char* LOG_FILE;

void init_reproducer_test(const char* path, const char* reprod_args) {

    //first check if file path is valid
    struct stat st;
    if(stat(path, &st) < 0 || !((st.st_mode & S_IFMT) == S_IFREG)) {
        fprintf(stderr, "Error while reading %s: ", path);
        perror("");
        exit(1);
    }

    // We need 2 more characters
    char path_ [strlen(path) + strlen(reprod_args) + 4];

    if (path_ == NULL) {
        //malloc failed, deal with it
        perror("Error");
        exit(1);
    }
    sprintf(path_, "./%s %s%c", path, reprod_args, '\0');
    IS_REPRODUCER = path_;
}

void init_temp_dirs(char* ir_path) {

    if(OUT_PATH == NULL) {
        //path is valid, we already checked
        OUT_PATH = dirname(ir_path);
    } else {
        //check if path is valid
        struct stat st;
        if(stat(OUT_PATH, &st) < 0 || !((st.st_mode & S_IFMT) == S_IFDIR)) {
            fprintf(stderr, "Error while reading %s: ", OUT_PATH);
            perror("");
        }
    }

    // add '/' to path if necessary
    if(OUT_PATH[strlen(OUT_PATH) - 1] != '/') {
        char out_path_new[strlen(OUT_PATH) + 1];
        sprintf(out_path_new,"%s%c%c", OUT_PATH, '/', '\0');
        OUT_PATH = out_path_new;
    }

    // move and rename initial test-case to temp directory    
    system("mkdir -p temp");
    char cp_cmd[strlen(ir_path) + 20];
    sprintf(cp_cmd, "cp %s temp/curr.ir%c", ir_path, '\0');
    system(cp_cmd);
}

ir_stats_t* get_ir_stats(char* path_to_file) {
    void* handle = dlopen("build/debug/statslib.so", RTLD_LAZY);
     if(!handle) {
        fprintf(stderr, "Error while loading library: %s\n", dlerror());
    }

    //clear errors
    dlerror();

    stats_func* get_stats;
    get_stats = (stats_func*) dlsym(handle, "get_ir_stats");
    const char* error = dlerror();
    if(error) {
        fprintf(stderr, "Cannot load symbol: %s", error);
        dlclose(handle);
    }

    return (*get_stats)(path_to_file);
}

void log_stats(FILE* stream, ir_stats_t* stats) {
    fprintf(stream, "\t# of nodes: %d\n", stats->node_n);
    fprintf(stream,"\t# of irgs: %d\n", stats->irg_n);
    fprintf(stream,"\t# of cf manipulations: %d\n", stats->cf_manips);
    fprintf(stream,"\t# of memory operations: %d\n", stats->mem_node_n);
    fprintf(stream,"\t# of types: %d\n\n", stats->type_n);
}

void init_logging() {
    FILE* log_file;
    char file_path[strlen(OUT_PATH) + 14];
    sprintf(file_path, "%sReduction.log%c", OUT_PATH, '\0');
    log_file = fopen(file_path, "a+");
    if(log_file == NULL) {
        fprintf(stderr, "Couldn't create log-file");
        exit(1);
    }
    fprintf(log_file, "Firmreduce -- Results\n\nInitial Test-case size:\n");
    log_stats(log_file, get_ir_stats("temp/curr.ir"));
    fclose(log_file);
}


void init(char* rep_path, char* reprod_args, char* ir_path) {
    init_reproducer_test(rep_path, reprod_args);
    init_temp_dirs(ir_path);
    init_logging();
}


void finish(ir_stats_t* final) {
    printf("final testcase size:\n");
    //TODO: print final statistics     
}

//execute shell script
int is_reproducer() {


    system(IS_REPRODUCER);
    return 1;
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

    while(!fixpoint) {
        
        }

        if(no_improvement >= PASSES_N || pass_failed >= PASSES_N) {
            fixpoint = 1;
        }
    

    /**
     * reduction is finished
     * print some nice things
     */
    printf("__________________________________________________________________________\n\n");
    printf("No further reduction possible.\n");
    printf("Total # of applied passes: %d\n\n", PASSES_APPLIED);
}

/**
 * function handles all itneraction w/ libstats object
 */
int has_improved() {

    ir_stats_t* old = get_ir_stats("temp/curr.ir");
    ir_stats_t* new = get_ir_stats("temp/temp.ir");

    int changed = (old->node_n != new->node_n
               || old->mem_node_n != new->mem_node_n 
               || old->cf_manips != new->cf_manips
               || old->type_n != new->type_n
               || old->type_n != new->type_n) ? 1 : 0;
    
    if (changed) {
        FILE* f = fopen(LOG_FILE, "a");
        log_stats(f, new);
        fclose(f);
    }
    return changed;
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
    /*
    ir_stats_t* final = reduce();
    finish(final);
    */
}