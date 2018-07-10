# GC Irgs

**Bug:** Pass gc_irgs fails on input 'input.ir 0 0' w/ the error message :

pass_gc_irgs: ir/ana/cgana.c:52: cg_get_call_n_callees: Assertion `is_Call(node) && node->attr.call.callee_arr' failed.
Aborted (core dumped)

Assertion fails bc callee_arr is not initialized --> missing calle analysis

**Start Reduction:**
- using script gc_irgs_callee_analysis_fail.sh
- had to stop halfway through
    - intermediate result: .ir-file: inter.ir
    - reproducer test on inter.ir is successful
- start reduction again on intermediate result
- Results:
    Result:
	# of nodes: 			24
	# of irgs: 				3
	# of cf manipulations: 	6
	# of memory operations: 1
	# of types: 			2055
    is still reproducer !!


added 2nd testcase, fails on '0 0'