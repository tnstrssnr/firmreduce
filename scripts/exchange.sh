#!/usr/bin/env bash

# don't mess w/ out variants -- make a backup
result=$(/home/tina/Documents/Bachelorarbeit/firmreduce/build/debug/passes/exe/pass_replace_loads $1 dump.ir 0 _main 0 2>&1 >/dev/null | grep -o "irg == get_irn_irg(nw)" | wc -l)
 # restore variant

if [ $result -eq 0 ]; then
    exit 0
else
    exit 1
fi