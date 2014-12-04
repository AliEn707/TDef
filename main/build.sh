#!/bin/sh
gcc simple.c ../*.c ../*.cpp -m32 -lm -lstdc++ -g -Wall -lpthread -o server