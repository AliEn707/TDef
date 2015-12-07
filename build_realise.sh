#!/bin/sh
uname -m && echo build
gcc bin/simple.c src/*.c -march=native -lm -Wall -lpthread -O3 -ffast-math -fgcse-sm -fgcse-las -fgcse-after-reload -fsigned-char -fgnu89-inline -o bin/server
