#!/usr/bin/env bash

result=$(/home/tina/Documents/Bachelorarbeit/firmreduce/build/debug/passes/exe/pass_libfirm_opt_ldst $1 dump.ir 0 main 1 | grep -o "set_Block_phis_: Assertion \`ir_resources_reserved(get_irn_irg(block)) & IR_RESOURCE_PHI_LIST' failed." | wc -l)
if [ $result -eq 0 ]; then
    exit 0
else
    exit 1
fi