# FirmReduce

## Build

**Make:**

Clone the repository recursively (to include libfirm. Alternatively place libfirm in a directory called libfirm in the source directory yourself) and call 'make' in the source directory. The results are put in a directory called 'build'.


## Adding passes

To add a pass put the corresponding executable file in the build/passes/dll directory.

Firmreduce calls the passes w/ the following arguments (in this order):
 - Path to .ir-file that should be reduced
 - Index of irg that the pass should be applied to, -1 if indicator variable is set to one
 - Indicator variable for reduction granularity:
    - 0 to reduce more aggressively
    - 1 to reduce more conservative
 - String identifier of the requested irg, if indicator variable=1

The variant that the pass produces should be saved to "temp/temp.ir"

Passes can be compiled using make by placing the source files in the src/passes directory and using the target 'make passes' (the target is also included in 'make' command).
If a pass has dependencies other than standard C libraries or libfirm, it should be compiled separately.

## Starting FirmReduce

To reduce a testcase, FirmReduce expects 2 input arguments: A path to a script, that returns 1 if the testcase is a bug reproducer and the path to the .ir-file of the testcase in question.
Optional parameters:
-o : Path to directory, where all output files should be stored. If none is specified, the current directory is used ass the output directory
-a : A String that will be given to the reproducer script as input parameter.

Example:

    firmreduce path/to/reproducer/script path/to/testcase -o path/to/output/dir -a "Parameters enclosed in quotation marks"