#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <dlfcn.h>
#include <string.h>

#include "passes/passes.h"
#include "logging.h"

pass_t* new_pass(char* ident, char* path) {

    pass_t* new_pass = malloc(sizeof(pass_t));
    new_pass->ident = ident;
    new_pass->path = path;
    return new_pass;

}

void init_passes_dynamic() {
    const char* pass_dir = "build/passes/dll";

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
    }

    passes = malloc(PASSES_N*sizeof(pass_t));
    dir = NULL;
    dir_ent = NULL;
    int i  = 0;

    if((dir = opendir(pass_dir)) != NULL) {

        while((dir_ent = readdir(dir)) != NULL) {
            if(dir_ent->d_name[0] != '.') {
                char* path = malloc(sizeof(dir_ent->d_name) + sizeof(pass_dir) + 2);
                sprintf(path, "%s/%s%c", pass_dir, dir_ent->d_name, '\0');
                passes[i] = new_pass(dir_ent->d_name, path);
                i++;
            }
        }
    }
}

int apply_pass(int i) {
    FILE* f = popen(passes[i]->path, "r");
    if(!f) {
        printf("Couldn't execute pass %s.\n", passes[i]->ident);
        exit(1);
    }
    pclose(f);
    char result = fgetc(f);
    fclose(f);
    log_text("Applying pass: ");
        log_text(passes[i]->ident);
        log_text("\t -- \t ");
        switch(result) {
            case 1:
                log_text("Successful\n");
                PASSES_APPLIED++;
                break;
            case 0:
                log_text("No improvement\n");
                break;
            case -1:
                log_text("Failed\n");
                break;
            default:
                break;
        }
    return result;
}