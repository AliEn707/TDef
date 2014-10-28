#!/bin/sh
gcc simple.c ../*.c -m32 -lm -g -Wall -lpthread -o server