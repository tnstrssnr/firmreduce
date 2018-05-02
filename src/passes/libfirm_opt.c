#include <firm.h>

void irg_apply_opt(ir_graph* irg) {
    opt_bool(irg);
    optimize_cf(irg);
    opt_if_conv(irg);
    // ...
}