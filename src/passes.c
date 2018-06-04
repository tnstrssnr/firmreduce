#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <dlfcn.h>
#include <string.h>
#include <limits.h>

#include "passes/passes.h"
#include "logging.h"

pass_t* new_pass(char* ident, char* path) {

    pass_t* new_pass = malloc(sizeof(pass_t));
    new_pass->ident = ident;
    new_pass->path = path;
    return new_pass;

}

void init_passes_dynamic() {
    const char* pass_dir = "build/passes/dll"; //TODO: what happens if we start program from other directory?

    DIR* dir;
    struct dirent* dir_ent;

    if((dir = opendir(pass_dir)) != NULL) {

        // count how many passes we can load 
        // TODO: find other way to count dir entries, instead of looping twice
        while((dir_ent = readdir(dir)) != NULL) {
            if(dir_ent->d_name[0] != '.') { // ignore hidden files
                PASSES_N++;
            }
        }
        closedir(dir);
    } else {
        fprintf(stderr, "Couldn't find pass directory\n");
        exit(1);
    }

    passes = malloc(PASSES_N*sizeof(pass_t));
    dir = NULL;
    dir_ent = NULL;
    int i  = 0;

    if((dir = opendir(pass_dir)) != NULL) {

        while((dir_ent = readdir(dir)) != NULL) {
            if(dir_ent->d_name[0] != '.') {
                char* path = malloc(sizeof(dir_ent->d_name) + sizeof(pass_dir) + 4);
                sprintf(path, "./%s/%s%c", pass_dir, dir_ent->d_name, '\0');
                passes[i] = new_pass(dir_ent->d_name, path);
                i++;
            }
        }
    }
    printf(":: %d passes found\n", PASSES_N);
}

/**
 * arguments:
 * path: path to ir-file that the pass should be applied to
 * i: index of pass
 * arg: index of irg to apply the pass to, -1 for random irg
 * ident: ident of irg
 */
int apply_pass(char* path, int i, int arg, char* ident) {

    char* path_;
    int status;
    int result;

    path_ = malloc(300);
    sprintf(path_, "%s %s %d%c", passes[i]->path, path, arg, '\0');

    status = system(path_);      
    result = -1;
    
    if(WIFEXITED(status)) {
        result = WEXITSTATUS(status);
    }
    
    log_text("\n");
    if(arg != -1) {
        char log_irg[strlen("Modifying irg \'\'  ") + strlen(ident) + 2];
        sprintf(log_irg, "Modifying irg \'%s\'%c", ident, '\0');
        log_text(log_irg);
        log_text(" -- ");
    }
    log_text("Applying pass: ");
    PASSES_APPLIED++;
        log_text(passes[i]->ident);
        log_text(" -- ");
        switch(result) {
            case 0:
                log_text("No improvement");
                PASSES_APPLIED++;
                break;
            case 1:
                log_text("Successful");
                break;
            default:
                log_text("Failed");            
                break;
        }
    free(path_);
    return result;
}