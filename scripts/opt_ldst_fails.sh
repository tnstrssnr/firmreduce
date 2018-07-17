cp temp/temp.ir temp/testfile.ir # don't mess w/ out variants -- make a backup
result=$(./build/debug/passes/exe/pass_libfirm_opt_ldst temp/temp.ir 0 0 2>&1 >/dev/null | grep -o "set_Block_phis_: Assertion \`ir_resources_reserved(get_irn_irg(block)) & IR_RESOURCE_PHI_LIST' failed." | wc -l)
mv temp/testfile.ir temp/temp.ir # restore variant

if [ $result -eq 0 ]; then
    exit 0
else
    exit 1
fi