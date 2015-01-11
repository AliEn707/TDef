#!/bin/sh
uname -m && echo build
gcc simple.c ../*.c -lm -g -Wall -lpthread -o server