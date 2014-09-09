#!/bin/sh
gcc grafic.c ../*.c -m32 -lGL -lGLU -lglut -lm -g -o "grafics"
gcc simple.c ../*.c -m32 -lm -g -Wall -lpthread -o simple