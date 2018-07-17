#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <dlfcn.h>
#include <string.h>
#include <limits.h>
#include <libgen.h>
#include <errno.h>
#include <stdbool.h>

#include "passes/passes.h"
#include "logging.h"

pass_t* new_pass(char* ident, char* path) {

    pass_t* new_pass = malloc(sizeof(pass_t));
    new_pass->ident = ident;
    new_pass->path = path;
    return new_pass;

}

void init_passes_dynamic(char* path) {
    bool absolute_path = 0;
    char* pass_dir;
    DIR* dir_ = opendir("/usr/local/bin/passes/");
    if (!dir_) {
        printf("Passes not in local\n");
        // directory does not exist. Try build directory
        size_t str_size = strlen(path) + strlen("/passes/exe/") + 1;
        pass_dir = malloc(str_size);
        snprintf(pass_dir, str_size, "%s/passes/exe/", path);
    } else {
        // directory exists. Look here for passes
        closedir(dir_);
        absolute_path = 1;
        pass_dir = "/usr/local/bin/passes/";
    }

    DIR* dir;
    struct dirent* dir_ent;
    printf("Pass directory: %s\n", pass_dir);
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
                char* pass_path;
                if(absolute_path) {
                    size_t str_size = strlen(dir_ent->d_name) + strlen(pass_dir) + 1;
                    pass_path = malloc(str_size);
                    snprintf(pass_path, str_size, "%s%s", pass_dir, dir_ent->d_name);
                } else {
                    size_t str_size = strlen(dir_ent->d_name) + strlen(pass_dir) + 3;
                    pass_path = malloc(str_size);
                    printf("Dirent name: %s\n", dir_ent->d_name);
                    snprintf(pass_path, str_size, "./%s%s", pass_dir, dir_ent->d_name);
                }
                
                passes[i] = new_pass(dir_ent->d_name, pass_path);
                printf("%s\n", passes[i]->path);
                i++;
            }
        }
    }
    printf(":: %d passes found\n", PASSES_N);
}

/**
 * arguments:
 * path: path to ir-file that the pass should be applied to
 * dump: path to file the transformed variant shuld be dumped in
 * i: index of pass
 * arg: index of irg to apply the pass to
 * reduce_individual: indicator, if pass should be applioed to whole irg or single node
 * ident: identifier of irg - can be NULL if reduction is aggressive
 */
int apply_pass(char* path, char* dump, int i, int reduce_individual, char* ident, int seed) {

    char* path_;
    int status;
    int result;

    path_ = malloc(strlen(passes[i]->path) + strlen(path) + strlen(dump) + strlen(ident) + 128); // TODO: how long can seed be ?
    
    sprintf(path_, "%s %s %s %d %s %d", passes[i]->path, path, dump, reduce_individual, ident, seed);
    status = system(path_);      
    result = -1;
    
    if(WIFEXITED(status)) {
        result = WEXITSTATUS(status);
    } 
    PASSES_APPLIED++;
    free(path_);
    return result;
}