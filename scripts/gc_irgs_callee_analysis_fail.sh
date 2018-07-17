cp temp/temp.ir temp/testfile.ir # don't mess w/ out variants -- make a backup
result=$(./build/debug/passes/exe/pass_gc_irgs temp/temp.ir 0 0 2>&1 >/dev/null | grep -o "is_Call(node) && node->attr.call.callee_arr" | wc -l)
mv temp/testfile.ir temp/temp.ir # restore variant
cparser temp/temp.ir
if [ $result -ne 0 -a $? -eq 0 ] ; then
    exit 1
else
    exit 0
fi

