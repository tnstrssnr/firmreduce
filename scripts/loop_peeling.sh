cp temp/temp.ir temp/testfile.ir # don't mess w/ out variants -- make a backup
result=$(./build/passes/dll/pass_libfirm_do_loop_peeling temp/temp.ir 2 0 2>&1 >/dev/null | grep -o "libFirm panic in init_analyze: loop optimization not implemented" | wc -l)
mv temp/testfile.ir temp/temp.ir # restore variant

if [ $result -eq 0 ]; then
    echo 0
else
    echo 1
fi