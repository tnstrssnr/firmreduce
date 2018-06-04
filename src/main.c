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

char* CURRENT_VARIANT = "temp/curr.ir";
char* TEMP_VARIANT = "temp/temp.ir";
char* STATS = "temp/stats";
char* LIBSTATS_PATH = "build/debug/libstats";

void init_reproducer_test(const char* path, const char* reprod_args) {

    //first check if file path is valid
    struct stat st;
    if(stat(path, &st) < 0 || !((st.st_mode & S_IFMT) == S_IFREG)) {
        fprintf(stderr, "Error while reading %s: ", path);
        perror("");
        exit(1);
    }

    // We need 4 more characters
    IS_REPRODUCER_SCRIPT = malloc(strlen(path) + strlen(reprod_args) + 17);

    if (IS_REPRODUCER_SCRIPT == NULL) {
        //malloc failed, deal with it
        perror("Error");
        exit(1);
    }
    sprintf(IS_REPRODUCER_SCRIPT, "./%s %s%c", path, reprod_args, '\0');
}

void init_temp_dirs(char* ir_path) {

    // path to dump results in
    if(OUT_PATH == NULL) {
        //use ir path as output directory -- is valid, we already checked
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
    
    // move and rename initial test-case to temp directory    
    system("mkdir -p temp");
    char cp_cmd[strlen(ir_path) + 17];
    sprintf(cp_cmd, "cp %s %s%c", ir_path, CURRENT_VARIANT, '\0');
    system(cp_cmd);
}

ir_stats_t* get_ir_stats(char* path_to_file, int dump) {

    /**
     * Since libfirm is invoked w/ every call to this function, it can't keep track of how often we've dumped the graphs before.
     * We use the counter to do that
     */
    static int counter = 0;
    ir_stats_t* stats = malloc(sizeof(ir_stats_t));

    char command[strlen(LIBSTATS_PATH) + strlen(path_to_file) + strlen(OUT_PATH) + strlen(STATS) + 5 + (counter/10)];

    sprintf(command, "%s %s %s %d %s %d%c", LIBSTATS_PATH, path_to_file, STATS, dump, OUT_PATH, counter, '\0');
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

    counter++;
    return stats;
}

void finish() {
    log_text("\n\n______________________________\n\n");
    log_text("No further reduction possible.\n\n");
    log_text("Result:\n");
    log_stats(get_ir_stats(CURRENT_VARIANT, 1));   

    char move_result[strlen(OUT_PATH) + strlen(CURRENT_VARIANT) + 15];
    sprintf(move_result, "cp %s %s/result.ir%c", CURRENT_VARIANT, OUT_PATH, '\0');
    system(move_result);

    //TODO: delete temp dir

    printf("\n:: Reduction finished -- Results dumped in %s\n", OUT_PATH);

    free(IS_REPRODUCER_SCRIPT);
    free(OUT_PATH);
}

void init(char* rep_path, char* reprod_args, char* ir_path) {
    init_reproducer_test(rep_path, reprod_args);
    init_temp_dirs(ir_path);
    init_logging(OUT_PATH);
    init_passes_dynamic();
    log_stats(get_ir_stats(CURRENT_VARIANT, 1));
}

//execute shell script
int is_reproducer() {
    FILE* f = popen(IS_REPRODUCER_SCRIPT, "r");
    if(!f) {
        printf("Couldn't execute reproducer script.\n");
        exit(1);
    }
    char result = fgetc(f);
    int int_result = atoi(&result);
    fclose(f);
    
    log_text(" -- Reproducer Test: ");
    log_result(int_result);

    return int_result;
}

void reduce_irp_level() {

    /**
     * Greedy search of new variants:
     * loop through passes, check if we get a 'better' variant of the irp
     *     if yes: save as current variant
     *     if no: discard
     * repeat until full iteraton w/o improvement
     */

    int failed = 0;
    int fixpoint = 0;
    ir_stats_t* stats = get_ir_stats(CURRENT_VARIANT, 0);
    int irg_n = stats->irg_n;
    char** ids = stats->irg_ids;

    printf(":: Start reduction on irp level\n");
    log_text("\nFirst reduction cycle: IRP Level\n");
    /*
    * first try being aggressive w/ reductions
    */
    while(!fixpoint) {
        printf(". ");

        int total_result;
        int result;
        for(int i = 0; i < irg_n; i++) {

            if(failed >= irg_n) {
                fixpoint = 1;
                break;
            }

            total_result = 0;
            system("cp temp/curr.ir temp/temp.ir");
            log_text("\n");
            log_text("Modifying irg \'\'");
            log_text(ids[i]);
            log_text("\'\' -- "); 
            for(int j = 0; j < PASSES_N; j++) {
                result = apply_pass(TEMP_VARIANT, j, i, ids[i]);
                total_result = (total_result == 1) ? 1 : result;
            }
            log_result(total_result);
            if(total_result != 1 || !is_reproducer()) {
                failed++;
                continue;
            }
            failed = 0;
            char replace[strlen(CURRENT_VARIANT) + strlen(TEMP_VARIANT) + 5];
            sprintf(replace, "cp %s %s%c", TEMP_VARIANT, CURRENT_VARIANT, '\0');
            system(replace);
        }
    }
    free(stats->irg_ids);
    free(stats);
}

void reduce_irg_level() {

    /**
     * Greedy search of new variants:
     * loop through passes, check if we get a 'better' variant of the irp
     *     if yes: save as current variant
     *     if no: discard
     * repeat until full iteraton w/o improvement
     */

    int failed = 0;
    int fixpoint = 0;
    ir_stats_t* stats = get_ir_stats(CURRENT_VARIANT, 0);
    int irg_n = stats->irg_n;
    char** ids = stats->irg_ids;

    printf(":: Start reduction on irg level\n");
    log_text("\nSecond reduction cycle: IRG Level\n");
    /*
    * first try being aggressive w/ reductions
    */
    while(!fixpoint) {
        printf(". ");

        int total_result; // indicates whether pass application was successful for at least 1 irg in the irp
        for(int j = 0; j < PASSES_N; j++) {
            if(failed >= PASSES_N) {
            fixpoint = 1;
            break;
            }

            total_result = 0;
            int result;
            for(int i = 0; i < irg_n; i++) {
                log_text("\n");
                log_text("Modifying irg \'\'");
                log_text(ids[i]);
                log_text("\'\' -- Applying pass: ");
                log_text(passes[j]->ident);
                log_text(" -- ");
                result = apply_pass(CURRENT_VARIANT, j, i, ids[i]); // apply pass j to irg i
                log_result(result);
                if(!(result == 1) || !is_reproducer()) {
                    result = 0;
                } else {
                    // we found a new smaller variant -- set as current
                    char replace[strlen(CURRENT_VARIANT) + strlen(TEMP_VARIANT) + 5];
                    sprintf(replace, "cp %s %s%c", TEMP_VARIANT, CURRENT_VARIANT, '\0');
                    system(replace);
                }
                total_result = (total_result) ? 1  : result;      
            }
            failed = (total_result == 1) ? 0 : failed + 1;
        }
    }
    free(stats->irg_ids);
    free(stats);
}

void reduce_node_level() {

    /**
     * Greedy search of new variants:
     * loop through passes, check if we get a 'better' variant of the irp
     *     if yes: save as current variant
     *     if no: discard
     * repeat until full iteraton w/o improvement
     */

    int failed = 0;
    int fixpoint = 0;
    int next_pass = 0;

    printf(":: Start reduction on node level\n");
    log_text("\n\nThird reduction cycle: IRN Level\n");
    /*
    * now apply passes to individual irgs
    */
    while(!fixpoint) {
        printf(". ");
        if(failed >= 5*PASSES_N) { // TODO: when is it best to terminate loop ?
            fixpoint = 1;
            continue;
        }

        int result = apply_pass(CURRENT_VARIANT, next_pass, -1, ""); // -1 --> pass will choose a random irg
        next_pass = (next_pass + 1) % PASSES_N;

        //printf("Pass result: %d\n", result);
        if(!(result == 1) || !is_reproducer()) {
          failed++;
          continue;
        } 

        // we found a new smaller variant -- set as current
        char replace[strlen(CURRENT_VARIANT) + strlen(TEMP_VARIANT) + 5];
        sprintf(replace, "cp %s %s%c", TEMP_VARIANT, CURRENT_VARIANT, '\0');
        system(replace);

        // reset counters
        failed = 0;
    }
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

    if(argv[optind] == NULL || argv[optind + 1] == NULL || argv[optind + 2] != NULL) {
        fprintf(stderr, "Received unexpected number of arguments\n");
        exit(1);
    } else {
        init(argv[optind], reprod_args, argv[optind + 1]);
    }
    reduce_irp_level();
    reduce_irg_level();
    reduce_node_level();
    finish();
}