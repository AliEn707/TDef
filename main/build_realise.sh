#!/bin/sh
gcc simple.c ../*.c -march=native -m32 -lm -Wall -lpthread -lstdc++ -O3 -ffast-math -o server
