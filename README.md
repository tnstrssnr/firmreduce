# FirmReduce

## Build

**Make:**

Clone the repository recursively (to include libfirm. Alternatively place libfirm in a directory called libfirm in the source directory yourself) and call 'make' in the source directory. The results are put in a directory called 'build'.


## Adding passes

To add a pass put the corresponding executable file in the build/passes/dll directory.

Firmreduce calls the passes w/ the following arguments:
 - Path to .ir-file that should be reduced
 - Index of irg that the pass should be applied to, -1 if indicator variable is set to one
 - Indicator variable for reduction granularity:
    - 0 to reduce more aggressively
    - 1 to reduce more conservative
 - String identifier of the requested irg, if indicator variable=1

Passes can be compiled using make by placing the source files in the src/passes directory and using the target 'make passes' (the target is also included in 'make' command).

If a pass has dependencies other than standard C libraries or libfirm, it should be compiled separately.