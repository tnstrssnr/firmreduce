cp temp/temp.ir temp/testfile.ir # don't mess w/ out variants -- make a backup
result=$(./build/debug/passes/exe/pass_libfirm_do_loop_peeling temp/temp.ir 1 0 2>&1 >/dev/null | grep -o "libFirm panic in init_analyze: loop optimization not implemented" | wc -l)
mv temp/testfile.ir temp/temp.ir # restore variant

if [ $result -eq 0 ]; then
    exit 0
else
    exit 1
fi