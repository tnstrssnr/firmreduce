#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <dlfcn.h>
#include <string.h>
#include <limits.h>
#include <libgen.h>

#include "passes/passes.h"
#include "logging.h"

pass_t* new_pass(char* ident, char* path) {

    pass_t* new_pass = malloc(sizeof(pass_t));
    new_pass->ident = ident;
    new_pass->path = path;
    return new_pass;

}

void init_passes_dynamic(char* path) {
    char pass_dir[strlen(path) + strlen("/passes/exe/") + 1];
    sprintf(pass_dir, "%s/passes/exe/", path, '\0');

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
                sprintf(path, "./%s/%s", pass_dir, dir_ent->d_name);
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
 * arg: index of irg to apply the pass to
 * reduce_individual: indicator, if pass should be applioed to whole irg or single node
 * ident: identifier of irg - can be NULL if reduction is aggressive
 */
int apply_pass(char* path, int i, int arg, int reduce_individual, char* ident) {

    char* path_;
    int status;
    int result;

    path_ = malloc(512);
    sprintf(path_, "%s %s %d %d %s 2>1&> /dev/null", passes[i]->path, path, arg, reduce_individual, ident);

    status = system(path_);      
    result = -1;
    
    if(WIFEXITED(status)) {
        result = WEXITSTATUS(status);
    } 
    PASSES_APPLIED++;
    free(path_);
    return result;
}