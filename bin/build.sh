#!/bin/sh
uname -m && echo build
gcc simple.c ../src/*.c -lm -g -Wall -lpthread -fsigned-char -fgnu89-inline -o server