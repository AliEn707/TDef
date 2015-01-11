#!/bin/sh
gcc simple.c ../src/*.c -march=native -lm -Wall -lpthread -O3 -ffast-math -o server
