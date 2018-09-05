#!/usr/bin/env bash

result=$(/home/tina/Documents/Bachelorarbeit/firmreduce/build/debug/passes/exe/pass_gc_irgs $1 dump.ir 0 _main 1 2>&1 >/dev/null | grep -o "is_Call(node) && node->attr.call.callee_arr" | wc -l)
if [ $result -eq 0 ]; then
    exit 0
else
    exit 1
fi