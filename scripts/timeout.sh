timout_limit=30s
testcase="cparser/build/debug/cparser -O0 -m32 crash1.ir -Icsmith/runtime"


timeout $timeout_limit $testcase 2> /dev/null

if [ $? -eq 124 ]; then 
    echo 0
else
    echo 1
fi
