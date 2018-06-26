cp temp/temp.ir temp/testfile.ir # don't mess w/ out variants -- make a backup
result=$(./build/passes/dll/pass_gc_irgs temp/temp.ir 0 0 2>&1 >/dev/null | grep -o "is_Call(node) && node->attr.call.callee_arr" | wc -l)
mv temp/testfile.ir temp/temp.ir # restore variant

if [ $result -eq 0 ]; then
    echo 0
else
    echo 1
fi

