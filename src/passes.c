#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <dlfcn.h>
#include <string.h>

#include "passes/passes.h"

pass_t* new_pass(char* ident, void* handle, pass_func* func) {

    pass_t* new_pass = malloc(sizeof(pass_t));
    new_pass->ident = ident;
    new_pass->handle = handle;
    new_pass->func = func;
    return new_pass;

}

void init_passes_dynamic() {

    const char* pass_dir = "build/passes/dll";
    const char* dll_dir = "./build/passes/dll/";

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
    passes = malloc(sizeof(pass_t) * PASSES_N);
    printf("%d\n", PASSES_N);
    DIR* dir_;
    struct dirent* entry;

    if((dir_ = opendir(pass_dir)) != NULL) {

        int curr_pass = 0;
        while ((entry = readdir(dir_)) != NULL) {
            
            if(entry->d_name[0] != '.') {
                char* dll_path = malloc(sizeof(dll_dir) + sizeof(entry->d_name) + 1);
                strcpy(dll_path, dll_dir);
                strcat(dll_path, entry->d_name);
                printf("%s\n", dll_path);
                void* handle;
                handle = dlopen(dll_path, RTLD_NOW);
                if(!handle) {
                    fprintf(stderr, "Error while loading pass: %s\n", dlerror());
                }

                //clear errors
                dlerror();

                int i = 0;
                while(entry->d_name[i] != '.') {
                    i++;
                }

                char* function_name = malloc(sizeof(char)*(i + 2));
                memcpy(function_name, entry->d_name, (i)*sizeof(char));
                function_name[i] = '\0';
                printf("%s\n", function_name);
                pass_func* func = (pass_func*) dlsym(handle, function_name);

                passes[curr_pass] = new_pass(function_name,handle, func);

                const char* error = dlerror();
                if(error) {
                    fprintf(stderr, "Cannot load symbol: %s", error);
                    dlclose(handle);
                    continue;
                }

                curr_pass++; 
            }
                  
        }
        closedir(dir_);

    } else {
        fprintf(stderr, "Error: pass directory not found\n");
        exit(1);
    }
}

void apply_pass(ir_prog* irp) {

    for(int j = 0; j < PASSES_N; j++) {

        pass_t* pass = passes[j];

        printf("Applying pass : %s\n", pass->ident);
        for(int i = 0; i < get_irp_n_irgs(); i++) {

            (pass->func)(get_irp_irg(i), NULL);
        }
    }
}