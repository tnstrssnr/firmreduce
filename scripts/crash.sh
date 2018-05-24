cparser temp/temp.ir $@
if [ $? -ne 0 ] ; then
    echo 1
else
    echo 0
fi