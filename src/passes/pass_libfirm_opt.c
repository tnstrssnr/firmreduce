#include <libfirm/firm.h>

/**
 * applies a bunch of Firm optimizations
 * TODO: clean-up
 */
void pass_libfirm_opt(ir_graph* irg, void* data) {
    opt_bool(irg);
    optimize_cf(irg);
    opt_if_conv(irg);
    do_loop_inversion(irg);
	optimize_reassociation(irg);
	optimize_load_store(irg);
	optimize_graph_df(irg);
	combo(irg);
	scalar_replacement_opt(irg);
	place_code(irg);
	optimize_reassociation(irg);
	optimize_graph_df(irg);
	opt_jumpthreading(irg);
	optimize_graph_df(irg);
	construct_confirms(irg);
	optimize_graph_df(irg);
	remove_confirms(irg);
	optimize_cf(irg);
	optimize_load_store(irg);
	optimize_graph_df(irg);
	combo(irg);
	place_code(irg);
optimize_cf(irg);
}