cp temp/temp.ir temp/testfile.ir # don't mess w/ out variants -- make a backup
result=$(./build/passes/dll/pass_libfirm_opt_blocks temp/temp.ir 1 0 2>&1 >/dev/null | grep -o "Verify warning: Block BB[13025:12](r_func_95[13017]): normal block must have at least 1 input" | wc -l)
mv temp/testfile.ir temp/temp.ir # restore variant

if [ $result -eq 0 ]; then
    echo 0
else
    echo 1
fi