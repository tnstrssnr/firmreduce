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

#include "ir_stats_structs.h"
#include "passes/passes.h"
#include "logging.h"

#define CONSERVATIVE 1
#define AGGRESSIVE 0

char* IS_REPRODUCER_SCRIPT;
char* OUT_PATH;
char* MAY_BE_INTERESTING;

char* CURRENT_VARIANT = "temp/curr.ir";
char* TEMP_VARIANT = "temp/temp.ir";
char* STATS = "temp/stats";
char* LIBSTATS_PATH;

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

    if (!IS_REPRODUCER_SCRIPT) {
        //malloc failed, deal with it
        perror("Error");
        exit(1);
    }
    sprintf(IS_REPRODUCER_SCRIPT, "./%s %s", path, reprod_args);
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
    MAY_BE_INTERESTING = malloc(strlen(OUT_PATH) + strlen("/may_be_interesting") + 2);
    sprintf(MAY_BE_INTERESTING, "%s/may_be_interesting", OUT_PATH);
    char make_dir[strlen(MAY_BE_INTERESTING) + strlen("mkdir -p ") + 1];
    sprintf(make_dir, "mkdir -p %s", MAY_BE_INTERESTING);
    system(make_dir);
    
    // move and rename initial test-case to temp directory    
    system("mkdir -p temp");
    char cp_cmd[strlen(ir_path) + 5];
    sprintf(cp_cmd, "cp %s %s", ir_path, CURRENT_VARIANT);
    system(cp_cmd);

    // re-use variable. Path to current and temp variant have same string length
    sprintf(cp_cmd, "cp %s %s", ir_path, TEMP_VARIANT);
    system(cp_cmd);

    // create fails file
    FILE* f = fopen("temp/fails", "w");
    fclose(f);
}

ir_stats_t* get_ir_stats(char* path_to_file, int dump, char* suffix) {

    ir_stats_t* stats = malloc(sizeof(ir_stats_t));

    char command[strlen(LIBSTATS_PATH) + strlen(path_to_file) + strlen(OUT_PATH) + strlen(STATS) + strlen(suffix) + 6];

    // execute libstats
    sprintf(command, "%s %s %s %d %s %s", LIBSTATS_PATH, path_to_file, STATS, dump, OUT_PATH, suffix);
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
    system("rm -r temp/");

    printf("\n:: Reduction finished -- Results dumped in %s\n", OUT_PATH);
}

//execute shell script
bool is_reproducer() {
    FILE* f = popen(IS_REPRODUCER_SCRIPT, "r");
    if(!f) {
        printf("Couldn't execute reproducer script.\n");
        exit(1);
    }
    char result = fgetc(f);
    bool bool_result = atoi(&result);
    fclose(f);

    return bool_result;
}

void init(char* program_path, char* rep_path, char* reprod_args, char* ir_path) {

    char* dir_ = dirname(program_path);
    LIBSTATS_PATH = malloc(strlen(dir_) + strlen("/libstats") + 1);
    sprintf(LIBSTATS_PATH, "%s/libstats", dir_);
    init_reproducer_test(rep_path, reprod_args);
    init_temp_dirs(ir_path);
    init_logging(OUT_PATH);
    init_passes_dynamic(dir_);
    log_stats(get_ir_stats(CURRENT_VARIANT, 1, "initial"));
}

/**
 * function tries to reduce the irp as much as possible w/ the given pass.
 */
bool reduce_irg_level(int pass) {

    bool saved = 0; // variable needed if pass fails
    bool achieved_reduction = 0;
    int failed = 0;

    ir_stats_t* stats = get_ir_stats(CURRENT_VARIANT, 0, "null");
    int irg_n = stats->irg_n;
    int irg_idx = 0;

    srand(time(NULL));
    while(failed < irg_n) {
        irg_idx = rand() % irg_n;

        int res = apply_pass(CURRENT_VARIANT, pass, irg_idx, AGGRESSIVE, "");

        if(res == 1) { // pass was successful
            if(is_reproducer()) {
                achieved_reduction = 1;
                failed = 0; // reset counter

                // we found a new smaller variant -- set as current
                char replace[strlen(CURRENT_VARIANT) + strlen(TEMP_VARIANT) + 5];
                sprintf(replace, "cp %s %s%c", TEMP_VARIANT, CURRENT_VARIANT, '\0');
                system(replace);
                continue;
            }
                // variant is no reproducer - write pass and irg to file, so we can try to reduce more conservatively later on
                failed++;
                FILE* f = fopen("temp/fails", "a");
                fprintf(f, "%d %s\n", pass, stats->irg_ids[irg_idx]);
                fclose(f);
                log_text("\n\t :: Reproducer test failed on irg \'");
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
    int res = apply_pass(CURRENT_VARIANT, pass, -1, CONSERVATIVE, irg_ident);
    log_result(res);
    if(res == 1 && is_reproducer()) {
       
        // we found a new smaller variant -- set as current
        char replace[strlen(CURRENT_VARIANT) + strlen(TEMP_VARIANT) + 5];
        sprintf(replace, "cp %s %s", TEMP_VARIANT, CURRENT_VARIANT);
        system(replace);
        return 1;
        
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

    srand(time(NULL));
    for(int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
    return arr;
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
 *      -a "args": Arguments that should be passed to reproducer script. Don't forget to add the ""
 */

    char* reprod_args = "";

    int i;
    while((i = getopt(argc, argv, "o:a:")) != -1) {
        switch(i) {
            case 'o':
                OUT_PATH = malloc(strlen(optarg)*sizeof(char) + 2);
                sprintf(OUT_PATH, "%s/%c", optarg, '\0');
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

    if(argv[optind] == NULL || argv[optind + 1] == NULL || argv[optind + 2] != NULL) {
        fprintf(stderr, "Received unexpected number of arguments\n");
        exit(1);
    } else {
        init(argv[0], argv[optind], reprod_args, argv[optind + 1]);
    }

    // check if input file is reproducer
    if(!is_reproducer()) {
        fprintf(stderr, "Reproducer test fails on input -- aborting reduction\n");
        exit(1);
    }

    // start reduction -- loop through passes in random order and try to reduce the irp as much as possible w/ each pass
    bool result = 1;
    while(result) {
        int* rand_permutation = get_shuffle(PASSES_N);
        result = 0;
        for(int i = 0; i < PASSES_N; i++) {
            int pass = rand_permutation[i];
            log_text("\nUsing pass: ");
            log_text(passes[pass]->ident);
            int pass_result = reduce_irg_level(pass);
            log_text(" -- ");
            log_result(pass_result);
            result = result || pass_result;
        }    
    }

    log_text("\n\n :: 2nd reduction cycle\n");
    // look at fail file: if it exists we try to reduce the logged irgs again, but this time more conservatively
    FILE* fails = fopen("temp/fails", "r");
    char* line = NULL;
    size_t size = 0;

    bool achieved_improvement = 1;
    while(achieved_improvement) { 
        achieved_improvement = 0;
        while(getline(&line, &size, fails) != -1) {
            int pass = atoi(strtok(line, " "));
            char* irg_ident = strtok(NULL, " ");
            int result = reduce_irn_level(irg_ident, pass);
            achieved_improvement = achieved_improvement || result;
        }
    }
    fclose(fails);
    finish();   
}