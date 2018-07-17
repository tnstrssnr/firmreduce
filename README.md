# FirmReduce

## Build

**Make:**

Clone the repository recursively (to include libfirm. Alternatively place libfirm in a directory called libfirm in the source directory yourself) and call 'make' in the source directory. The results are put in a directory called 'build'.

If you want to install firmReduce use the target 'make install'.


## Adding passes

**Loading passes:**

Firmreduce looks for passes in the following directories in this order:
 - /usr/local/bin/passes
 - /path/to/firmreduce/src/dir/build/debug/passes/exe
The passes are loaded from the first directory that is found. To add additional passes, put them in a directory where firmReduce will find them.

**Pass interface:**

Each pass is its own executable file.
Firmreduce calls the passes w/ the following arguments (in this order):
 - Path to .ir-file that should be reduced
 - Path to file that the result should be dumped to
 - Indicator variable for reduction granularity:
    - 0 to reduce more aggressively
    - 1 to reduce more conservative
 - String identifier of the requested irg
 - Seed for random number generation (use this to make reduction reproducible)

Passes can be compiled using make by placing the source files in the src/passes directory and using the target 'make passes' (the target is also included in 'make' command).
If a pass has dependencies other than standard C libraries or libfirm, it should be compiled separately.

## Starting FirmReduce

To reduce a testcase, FirmReduce expects 2 input arguments: A path to a script, that returns 1 if the testcase is a bug reproducer and the path to the .ir-file of the testcase in question.
Optional parameters:
-o : Path to directory, where all output files should be stored. If none is specified, the current directory is used ass the output directory
-s : An integer used as the seed for random number generation in the program

Example:

    firmreduce path/to/reproducer/script path/to/testcase -o path/to/output/dir -a "Parameters enclosed in quotation marks" -s seed