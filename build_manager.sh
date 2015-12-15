#!/bin/sh
uname -m && echo build
gcc bin/manager.c bin/daemon.c bin/updater.c src/t_sem.c -g -Wall -lpthread -fsigned-char -o bin/manager