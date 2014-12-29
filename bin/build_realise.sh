#!/bin/sh
gcc simple.c ../*.c -march=native -m32 -lm -Wall -lpthread -O3 -ffast-math -o server
