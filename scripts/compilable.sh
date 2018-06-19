cparser temp/temp.ir $@ > /dev/null
if [ $? -eq 0 ] ; then
    exit 1
else
    exit 0
fi