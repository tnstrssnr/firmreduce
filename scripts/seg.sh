#!/usr/bin/env bash
cparser $1 -O0 --target=x86_64-linux-gnu -funroll-loops
if [ $? -eq 139 ] ; then
    exit 1
else
    exit 0
fi