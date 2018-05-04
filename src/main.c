#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>
#include <libfirm/firm.h>

#include "ir_stats.h"
#include "passes/passes.h"

/**
 * path to shell script, that checks if irg is still a reproducer
 */
const char* IS_REPRODUCER;

/**
 * path, where all (temp-)files are dumped
 */
const char* OUT_PATH;

/**
 * current variant
 */
int variant_n = 1;

/**
 * http://www.strudel.org.uk/itoa/
 */
char* itoa(int val, int base){
	
	static char buf[32] = {0};
	int i = 30;
	
	for(; val && i ; --i, val /= base)	
		buf[i] = "0123456789abcdef"[val % base];	
	return &buf[i+1];
}

void init_reproducer_test(const char* path) {

    //first check if file path is valid
    struct stat st;
    if(stat(path, &st) < 0 || !((st.st_mode & S_IFMT) == S_IFREG)) {
        fprintf(stderr, "Error while reading %s: ", path);
        perror("");
        exit(1);
    }

    // We need 2 more characters
    char* path_ = (char*) malloc(strlen(path) + 2);

    if (path_ == NULL) {
        //malloc failed, deal with it
        perror("Error");
        exit(1);
    }

    strcpy(path_, "./");
    strcat(path_, path);
    IS_REPRODUCER = path_;
}

char* get_io_filename() {
    char* variant = itoa(variant_n, 10);

    // TODO: don't use itoa
    char* path = malloc(sizeof(OUT_PATH) + sizeof(variant) + 8);
    strcpy(path, OUT_PATH);
    strcat(path, "temp_");
    strcat(path, variant);
    strcat(path, ".ir");
    
    return path;
}

void export_variant() {
    char* path = get_io_filename();
    
    if(ir_export(path)) {
        fprintf(stderr, "Error while exporting irp.\n");
        perror("");
        exit(1);
    }
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

    // move and rename initial test-case to out directory    
    export_variant();
}


void init(char* rep_path, char* ir_path) {
    ir_init();
    init_passes_dynamic();
    init_reproducer_test(rep_path);

    if(ir_import(ir_path)) {
        fprintf(stderr, "Error while reading test-case file\n");
        exit(1);
    }

    init_temp_dirs(ir_path);
}


void finish(ir_stats_t* final) {
    printf("final testcase size:\n");
    print_stats(final);
    ir_set_dump_path(OUT_PATH);
    
    for(int i = 0; i < get_irp_n_irgs(); i++) {
        dump_ir_graph(get_irp_irg(i), "");
    }

    //TODO: print final statistics       
}

//execute shell script
int is_reproducer() {

    /**
     * the script should return 0 iff the tested variant is still a reproducer
     */
    //return system(IS_REPRODUCER);
    return 1;
}

int  is_valid() {
    // see if variant is a valid irp
    for(int i = 0; i < get_irp_n_irgs(); i++) {   
        if(!irg_verify(get_irp_irg(i))) {
                return 0;
        }
    }
    return 1;
}

ir_stats_t* reduce() {

    //get size and stuff for input graph
    
    ir_stats_t* curr_stats = get_ir_stats(variant_n);
    //ir_stats_t* init_stats = get_ir_stats(variant_n);

    printf("Initial testcase size:\n");
    print_stats(curr_stats);
    variant_n++;

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
        apply_pass(get_irp());
        ir_stats_t* new_variant = get_ir_stats(variant_n);
        print_stats(new_variant);
        if(is_valid()) { //test if variant is a valid irp

            ir_set_dump_path(OUT_PATH);
            for(int i = 0; i < get_irp_n_irgs(); i++) {
                dump_ir_graph(get_irp_irg(i), "");
            }
            
            if(is_reproducer() && (compare_stats(curr_stats, new_variant)->ident == variant_n)) { // test if variant is better than before and still a reproducer
                export_variant();
                free(curr_stats);
                curr_stats = new_variant;
                variant_n++;
                pass_failed = 0;
                no_improvement = 0; // we need to do at least 1 more round of passes to find fixpoint
                printf("A new smaller variant was found!\n");
                continue;
            } else {
                no_improvement++;
            }

        } else { // pass failed to produce valid irp
            pass_failed++;
        }

        if(no_improvement >= PASSES_N || pass_failed >= PASSES_N) {
            fixpoint = 1;
        }
    }

    /**
     * reduction is finished
     * print some nice things
     */
    printf("__________________________________________________________________________\n\n");
    printf("No further reduction possible.\n");
    printf("Total # of applied passes: %d\n\n", PASSES_APPLIED);
    return curr_stats;
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
 */

    int i;
    while((i = getopt(argc, argv, "o:")) != -1) {
        switch(i) {
            case 'o':
                OUT_PATH = optarg;
                break;
            case '?':
            default:
                fprintf(stderr, "Error while parsing arguments\n");
                exit(1);
        }
    }

    if(argv[optind] == NULL || argv[optind + 1] == NULL || argv[optind + 2] != NULL) {
        fprintf(stderr, "Received unexpected number of arguments\n");
        exit(1);
    } else {
        init(argv[optind], argv[optind + 1]);
    }

    ir_stats_t* final = reduce();  
    finish(final);

}