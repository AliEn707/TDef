#include "manager.h"
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>

int main(int argc, char **argv) {
	char s[100];
	InitWorkThread();
	scanf("%s", s);
	DestroyWorkThread();
	sleep(1);
	return 0;
}
