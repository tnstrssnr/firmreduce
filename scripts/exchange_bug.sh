cp temp/temp.ir temp/testfile.ir # don't mess w/ out variants -- make a backup
result=$(./build/debug/passes/exe/pass_replace_loads temp/temp.ir 0 0 2>&1 >/dev/null | grep -o "irg == get_irn_irg(nw)" | wc -l)
mv temp/testfile.ir temp/temp.ir # restore variant

if [ $result -eq 0 ]; then
    exit 0
else
    exit 1
fi