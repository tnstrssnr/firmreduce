#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>
#include <dlfcn.h>
#include <assert.h>
#include <time.h>
#include <stdbool.h>
#include <argp.h>

#include "ir_stats_structs.h"
#include "passes/passes.h"
#include "logging.h"

#define AGGRESSIVE 0
#define CONSERVATIVE 1

// paths to other modules
char* IS_REPRODUCER_SCRIPT;
char* LIBSTATS_PATH;

// output
char* OUT_PATH;
char* MAY_BE_INTERESTING;

// temporary files
char TEMP_DIR[] = "/tmp/firmreduce_temp.XXXXXX";
char* CURRENT_VARIANT;
char* TEMP_VARIANT;
char* STATS;
char* FAILS;

// options for argument parser
const char* argp_program_version = "firmreduce 1.0";

static struct argp_option options[] = {
  {"output",  'o', "PATH",      0,  "Write output to PATH instead of current working directory" },
  {"seed",    's', "SEED",      0,  "Enter seed for pseudorandom number generation" },
  { 0 }
};

static char args_doc[] = "/PATH/TO/SCRIPT PATH/TO/TESTCASE";

static char doc[] =
  "FirmReduce -  Benutzung auf eigene Gefahr. Eltern haften für ihre Kinder.";

// struct to save all command line arguments that we get
typedef struct arguments {
    char* args[2];
    int s;
} arguments;

/*
* Program expects 2 arguments:
*     1. path to .sh file -> interestingness test for bug
*     2. path to .ir file of input test-case
* 
* optional arguments:
*     -o path/to/dir: Expects path to directory, where temp files and reduced test-cases are dumped
*      If no path is specified, path of input test-case will be used
* 
*      -s seed: seed for generating random number sequence. Of none is specified a timestamp will be used as seed
*/
static int parse_opt(int key, char* arg, struct argp_state* state) {
    struct arguments* arguments = state->input;
    
    switch(key) {
        case 'o':
            OUT_PATH = arg;
            break;
        case 's':
            arguments->s = atoi(arg);
            break;
        case ARGP_KEY_ARG:
            if(state->arg_num >= 2) {
                argp_usage(state);
            }
            arguments->args[state->arg_num] = arg;
            break;
        case ARGP_KEY_END:
            if(state->arg_num < 1) {
                argp_usage(state);
            }
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};


void init_reproducer_test(const char* path, const char* reprod_args) {

    //first check if file path is valid
    struct stat st;
    if(stat(path, &st) < 0 || !((st.st_mode & S_IFMT) == S_IFREG)) {
        fprintf(stderr, "Error while reading %s: ", path);
        perror("");
        exit(1);
    }

    // We need 4 more characters
    size_t str_size = strlen(path) + strlen(reprod_args) + 4;
    IS_REPRODUCER_SCRIPT = malloc(str_size);

    if (!IS_REPRODUCER_SCRIPT) {
        //malloc failed, deal with it
        perror("Error");
        exit(1);
    }
    snprintf(IS_REPRODUCER_SCRIPT, str_size, "./%s %s", path, reprod_args);
}

void init_temp_dirs(char* ir_path) {

    // path to dump results in
    if(!OUT_PATH) {
        //use current directory as output directory
        OUT_PATH = "./";
    } else {
        //check if path is valid
        struct stat st;
        if(stat(OUT_PATH, &st) < 0 || !((st.st_mode & S_IFMT) == S_IFDIR)) {
            fprintf(stderr, "Error while reading %s: ", OUT_PATH);
            perror("");
        }
    }

    // add directory to store variants that may of interest, but don't help the current reduction
    size_t str_size = strlen(OUT_PATH) + strlen("/may_be_interesting") + 1;
    MAY_BE_INTERESTING = malloc(str_size);
    snprintf(MAY_BE_INTERESTING, str_size, "%s/may_be_interesting", OUT_PATH);

    str_size = strlen(MAY_BE_INTERESTING) + strlen("mkdir -p ") + 1;
    char make_dir[str_size];
    snprintf(make_dir, str_size, "mkdir -p %s", MAY_BE_INTERESTING);
    system(make_dir);
    


    // create temp files
    char* temp = mkdtemp(TEMP_DIR);
    if(!temp) {
         fprintf(stderr, "Error while creating temp files");
        exit(1);
    }
    CURRENT_VARIANT = malloc(strlen(temp) + strlen("/curr.ir") + 1);
    snprintf(CURRENT_VARIANT, strlen(temp) + strlen("/curr.ir") + 1, "%s/curr.ir", temp);
    TEMP_VARIANT = malloc(strlen(temp) + strlen("/temp.ir") + 1);
    snprintf(TEMP_VARIANT, strlen(temp) + strlen("/temp.ir") + 1, "%s/temp.ir", temp);
    STATS = malloc(strlen(TEMP_DIR) + strlen("/stats") + 1);
    snprintf(STATS, strlen(TEMP_DIR) + strlen("/stats") + 1, "%s/stats", TEMP_DIR);
    FAILS = malloc(strlen(TEMP_DIR) + strlen("/fails") + 1);
    snprintf(FAILS, strlen(TEMP_DIR) + strlen("/fails") + 1, "%s/fails", TEMP_DIR);
    
    // move and rename input testcase
    str_size = strlen(ir_path) + strlen(CURRENT_VARIANT) + 5;
    char* cp_cmd = malloc(str_size);
    snprintf(cp_cmd, str_size, "cp %s %s", ir_path, CURRENT_VARIANT);
    system(cp_cmd);

    // re-use variable. Path to current and temp variant have same string length
    snprintf(cp_cmd, str_size, "cp %s %s", ir_path, TEMP_VARIANT);
    system(cp_cmd);

    // open fails file once, to make sure it exists
    FILE* f = fopen(FAILS, "w");
    fclose(f);
}

ir_stats_t* get_ir_stats(char* path_to_file, int dump, char* suffix) {

    ir_stats_t* stats = malloc(sizeof(ir_stats_t));
    size_t str_size = strlen(LIBSTATS_PATH) + strlen(path_to_file) + strlen(STATS) + strlen(OUT_PATH) + strlen(suffix) + 7; // 5 spaces + 1 char for 'dump' variable + 1 nul terminator
    char* command = malloc(str_size);

    // execute libstats
    snprintf(command, str_size, "%s %s %s %d %s %s", LIBSTATS_PATH, path_to_file, STATS, dump, OUT_PATH, suffix);
    system(command);

    // read the stats from the output file
    FILE* f = fopen(STATS, "r");
    char* line = NULL;
    size_t size = 0;
    getline(&line, &size, f);

    stats->node_n = atoi(strtok(line, " "));
    stats->mem_node_n = atoi(strtok(NULL, " "));
    stats->cf_manips = atoi(strtok(NULL, " "));
    stats->type_n = atoi(strtok(NULL, " "));
    stats->irg_n = atoi(strtok(NULL, " "));

    stats->irg_ids = malloc(sizeof(char*)*stats->irg_n);
    getline(&line, &size, f);
    stats->irg_ids[0] = strtok(line, " ");
    for(int i = 1; i < stats->irg_n; i++) {
        stats->irg_ids[i] = strtok(NULL ," ");
    }
    fclose(f);
    return stats;
}

/**
 * Log results, move output to out_dir, clean up temp files
 */
void finish() {
    log_text("\n\n______________________________\n\n");
    log_text("No further reduction possible.\n\n");
    log_text("Result:\n");
    log_stats(get_ir_stats(CURRENT_VARIANT, 1, "final"));   

    char move_result[strlen(OUT_PATH) + strlen(CURRENT_VARIANT) + strlen("/result.ir") + 5];
    sprintf(move_result, "cp %s %s/result.ir", CURRENT_VARIANT, OUT_PATH);
    system(move_result);

    unlink(TEMP_DIR);
    rmdir(TEMP_DIR);

    printf("\n:: Reduction finished -- Results dumped in %s\n", OUT_PATH);
}

//execute shell script
int is_reproducer() {
    int status = system(IS_REPRODUCER_SCRIPT);
    int result = -1;
    if(WIFEXITED(status)) {
        result = WEXITSTATUS(status);
    }
    return result;
}

void init(char* program_path, char* rep_path, char* ir_path, int seed) {

    char* dir_ = dirname(program_path);
    // look for libstats file
    char* libstats = "/usr/local/bin/libstats";
    if( access( libstats , X_OK ) != -1 ) {
        LIBSTATS_PATH = libstats;
    } else {
        size_t str_size = strlen(dir_) + strlen("/libstats") + 1;
        LIBSTATS_PATH = malloc(str_size);
        snprintf(LIBSTATS_PATH, str_size, "%s/libstats", dir_);
    }
    
    if (access(LIBSTATS_PATH, X_OK) == -1) {
        fprintf(stderr, "Couldn't find stats executable. Aborting\n");
        exit(1);
    }

    init_temp_dirs(ir_path);
    init_reproducer_test(rep_path, TEMP_VARIANT);
    init_logging(OUT_PATH, seed);
    init_passes_dynamic(dir_);
    log_stats(get_ir_stats(CURRENT_VARIANT, 1, "initial"));
}

 // we found a new smaller variant -- set as current
void replace_variant() {
    size_t str_size = strlen(CURRENT_VARIANT) + strlen(TEMP_VARIANT) + 5;
    char replace[str_size];
    snprintf(replace, str_size, "cp %s %s", TEMP_VARIANT, CURRENT_VARIANT);
    system(replace);
}

/**
 * function tries to reduce the irp as much as possible w/ the given pass.
 */
bool reduce_irg_level(int pass) {

    bool saved = 0; // variable needed if pass fails --> we want to save it to 'may_be_interesting' folder only once
    bool achieved_reduction = 0;
    int failed = 0;

    ir_stats_t* stats = get_ir_stats(CURRENT_VARIANT, 0, "null");
    int irg_n = stats->irg_n;
    int irg_idx = 0;
    int i = 0; // count how namy times pass gets applied to a random irg -- do max. 2*irg_n

    while(failed < irg_n && i < 2*irg_n ) {
        i++;
        irg_idx = rand() % irg_n; // use irg_idx entry in identifier array -- does not correspond w/ irg_nr when irp is loaded by pass

        int res = apply_pass(CURRENT_VARIANT, TEMP_VARIANT, pass, AGGRESSIVE, stats->irg_ids[irg_idx], rand());
        if(res == 1) { // pass was successful
            if(is_reproducer()) {
                achieved_reduction = 1;
                failed = 0; // reset counter

                replace_variant();
                continue;
            }
            
            if(!saved) { // only save once
                char file_name[256 +  strlen("cp  ") + strlen(CURRENT_VARIANT) + strlen(MAY_BE_INTERESTING) + 1];
                time_t t = time(NULL);
                struct tm tm = *localtime(&t);
                sprintf(file_name, "cp %s %s/%d%d%d-%d%d%d.ir", CURRENT_VARIANT, MAY_BE_INTERESTING, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                system(file_name);
                saved = 1;
            }
                // variant is no reproducer - write pass and irg to file, so we can try to reduce more conservatively later on
                failed++;
                FILE* f = fopen(FAILS, "a");
                fprintf(f, "%d %s\n", pass, stats->irg_ids[irg_idx]);
                fclose(f);
                log_text("\n\t :: interestingness test failed on irg \'");
                log_text(stats->irg_ids[irg_idx]);
                log_text("\'");

        } else if (res != 0) {
            // the pass failed. Usually this is an indication, that we found an interesting test-case for libFirm. We save the variant
            // that caused the pass to fail (i.e. the input variant of the pass) for later
            log_text("\n\t :: Pass failed on irg \'");
            log_text(stats->irg_ids[irg_idx]);
            log_text("\'");

            if(!saved) { // only save once
                char file_name[256 +  strlen("cp  ") + strlen(CURRENT_VARIANT) + strlen(MAY_BE_INTERESTING) + 1];
                time_t t = time(NULL);
                struct tm tm = *localtime(&t);
                sprintf(file_name, "cp %s %s/%d%d%d-%d%d%d.ir", CURRENT_VARIANT, MAY_BE_INTERESTING, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                system(file_name);
                saved = 1;
            }
        }
        failed++;
    }
    return achieved_reduction;
}

bool reduce_irn_level(char* irg_ident, int pass) {
    log_text("\nUsing pass: ");
    log_text(passes[pass]->ident);
    log_text(" -- Trying irg \'");
    log_text(irg_ident);
    log_text("\' -- ");
    int res = apply_pass(CURRENT_VARIANT, TEMP_VARIANT, pass, CONSERVATIVE, irg_ident, rand());
    if(res == 1 && is_reproducer()) {
        log_result(res);
        replace_variant();
    }
    return 0;           
}

/**
 * Uses the Fisher-Yates shuffle to create a random permutation of the array {0,...,size-1}
 * This is used to randomize the order in which passes are applied to the testcase
 */
int* get_shuffle(int size) {
    int* arr = malloc(size* sizeof(int));

    for(int i = 0; i < size; i++) {
        arr[i] = i;
    }

    for(int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
    return arr;
}

int main(int argc, char** argv) {

    struct arguments arguments;

    // default values
    OUT_PATH = "./";
    arguments.s = time(NULL);

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    // initialize pseudorandom number generation w/ seed
    printf("Using seed %d for pseudorandom number generation.\n", arguments.s);
    srand(arguments.s);
    init(argv[0], arguments.args[0], arguments.args[1], arguments.s);

    // check if input file is reproducer
    if(!is_reproducer()) {
        fprintf(stderr, "interestingness test fails on input -- aborting reduction\n");
        exit(1);
    }

    // start reduction -- loop through passes in random order and try to reduce the irp as much as possible w/ each pass
    bool result = 1;
    while(result) {
        int* rand_permutation = get_shuffle(PASSES_N);
        result = 0;
        for(int i = 0; i < PASSES_N; i++) {
            int pass = rand_permutation[i];
            int pass_result = reduce_irg_level(pass);
            result = result || pass_result;
            log_text("\nUsing pass: ");
            log_text(passes[pass]->ident);
            log_text(" -- ");
            log_result(pass_result);
            log_text("\n");
        }    
    }

    log_text("\n\n :: 2nd reduction cycle\n");
    // look at fail file: if it exists we try to reduce the logged irgs again, but this time more conservatively
    FILE* fails = fopen(FAILS, "r");
    char* line = NULL;
    size_t size = 0;

    bool achieved_improvement = 1;
    while(achieved_improvement) { 
        achieved_improvement = 0;
        while(getline(&line, &size, fails) != -1) {
            int pass = atoi(strtok(line, " "));
            char* irg_ident = strtok(NULL, "\n");
            int result = reduce_irn_level(irg_ident, pass);
            achieved_improvement = achieved_improvement || result;
        }
    }
    fclose(fails);
    finish(); 
}