cp temp/temp.ir temp/testfile.ir # don't mess w/ out variants -- make a backup
result=$(./build/debug/passes/exe/pass_libfirm_opt_blocks temp/temp.ir 19 0 2>&1 >/dev/null | grep -o "normal block must have at least 1 input" | wc -l)
mv temp/testfile.ir temp/temp.ir # restore variant

if [ $result -eq 0 ]; then
    exit 0
else
    exit 1
fi