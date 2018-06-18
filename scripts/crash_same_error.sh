NEW=$(cparser temp/temp.ir -O0 -fivopts 2>&1)

OLD="$(cparser examples/20180611-1/input.ir -O0 -fivopts 2>&1)"

if [ "$OLD" == "$NEW" ] ; then
  echo 1;
else
  echo 0;
fi