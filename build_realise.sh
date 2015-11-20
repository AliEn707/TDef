#!/bin/sh
uname -m && echo build
gcc bin/simple.c src/*.c -march=native -lm -Wall -lpthread -O3 -ffast-math -fsigned-char -fgnu89-inline -o bin/server
