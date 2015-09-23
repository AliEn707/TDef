#!/bin/sh
gcc manager.c daemon.c updater.c ../src/t_sem.c -g -Wall -lpthread -fsigned-char -o manager