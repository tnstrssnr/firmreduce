timout_limit=5
timeout $timeout_limit cparser temp/temp.ir $@

if [ $? -eq 124 ]; then 
    echo 0
else
    echo 1
fi
