#!/bin/sh
gcc manager.c daemon.c updater.c -g -Wall -lpthread -fsigned-char -o manager