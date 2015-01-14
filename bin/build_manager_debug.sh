#!/bin/sh
gcc manager.c perf_test.c debug.c -g -Wall -lpthread -fsigned-char -o manager