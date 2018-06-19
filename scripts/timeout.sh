timout_limit=5
timeout $timeout_limit cparser temp/temp.ir $@

if [ $? -eq 124 ]; then 
    exit 0
else
    exit 1
fi
