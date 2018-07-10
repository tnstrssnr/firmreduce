# Mini-Compilable

**Script used:** compilable.sh (no further arguments)

**Input used:** examples/20180611-1/input.ir

**Output:** result.ir is compilable using cparser result.ir

**libFirm:** no bugs were added

**Goal:** When reducing crash-bugs, firmreduce kept producing variants that fail at a different assertion than the input. To see, where this error gets introduced,
we reduce the input test-case as much as possible, while keeping it compilable.

**Result:** We see a number of passes, that fail the reproducer test during reduction, hence this is where a crash-bug is introduced. This especially concerns non-destructive libfirm-passes, as they do nothing but executing a firm compiler-optimization: A crash bug introduced by one of these passes indicates a bug in the corresponding optimization.

