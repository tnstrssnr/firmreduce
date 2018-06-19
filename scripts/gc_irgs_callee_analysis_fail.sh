cp temp/temp.ir temp/testfile.ir # don't mess w/ out variants -- make a backup
./build/passes/dll/pass_gc_irgs temp/temp.ir 0 0 2&> temp/test.txt
mv temp/testfile.ir temp/temp.ir # restore variants
grep -c -w "node->attr.call.callee_arr" temp/test.txt

if [ $? -eq 0 ] ; then
    exit 0
else
    exit 1
fi