#!/usr/bin/env bash
result=$(cparser $1 -O0 --target=x86_64-linux-gnu -fbool 2>&1 >/dev/null | grep -o "pn == pn_Load_M" | wc -l)
if [ $result -eq 0 ] ; then
    exit 0
else
    exit 1
fi