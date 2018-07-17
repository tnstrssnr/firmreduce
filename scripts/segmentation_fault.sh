result=$(cparser temp/temp.ir $@ 2>&1 >/dev/null | grep -o "Segmentation fault" | wc -l)

if [ $result -eq 0 ]; then
    exit 0
else
    exit 1
fi