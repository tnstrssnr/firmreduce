NEW=$(cparser temp/temp.ir -O0 -fivopts 2>&1)

OLD="$(cparser examples/20180611-1/input.ir -O0 -fivopts 2>&1)"

if [ "$OLD" == "$NEW" ] ; then
  exit 1;
else
  exit 0;
fi