# Loop Peeling

- pass 'pass_libfirm_do_loop_peeling' fails w/ error msg "libFirm panic in init_analyze: loop optimization not implemented"
- input parameter for pass: 2 0

- 2nd input file: independant from first, but has same bug
- invoke w/ parameters "1 0"