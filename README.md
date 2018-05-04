# FirmReduce

## Build

**Make:**

Clone the repository recursively (to include libfirm. Alternatively place libfirm in a directory called libfirm in the source directory yourself) and call 'make' in the source directory. The results are put in a directory called 'build'.




## Adding passes

To add a pass put the corresponding .so file in the build/passes/dll directory.

FirmReduce expects the pass to have a 

    `void $(pass_name)(ir_graph* irg)`

function, where $(pass_name) is the name of the .so file. This function will be called to apply the pass to the irg.

Passes can be compiled using make by placing the source files in the src/passes directory and using the target 'make passes' (the target is also included in 'make' command).

If a pass has dependencies other than standard C libraries or libfirm, it should be compiled separately.