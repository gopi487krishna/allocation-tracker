#!/bin/bash
clang  -S -g -emit-llvm -fno-discard-value-names   -flegacy-pass-manager -Xclang -load -Xclang /home/cooldev/teeny-linux/allocation-tracker/build/allocation_tracer/libAllocationTracer.so example_prog.c
clang -g -gdwarf-4 -fno-discard-value-names   -flegacy-pass-manager -Xclang -load -Xclang /home/cooldev/teeny-linux/allocation-tracker/build/allocation_tracer/libAllocationTracer.so example_prog.c
