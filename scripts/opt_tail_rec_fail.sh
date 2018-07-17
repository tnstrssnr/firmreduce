cp temp/temp.ir temp/testfile.ir
res_1=$(cparser temp/temp.ir 2>&1 )
./build/passes/dll/pass_libfirm_opt_tail_rec temp/temp.ir 0 0
res_2=$(cparser temp/temp.ir 2>&1 )
mv temp/testfile.ir temp/temp.ir

if [ "$res_1" = "$res_2" ] ; then
    exit 0
else
    exit 1
fi