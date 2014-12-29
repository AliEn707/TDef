#include "manager.h"
#include "perf_test.h"
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>

int main(int argc, char **argv) {
	char s[100];
	if (argc>1)
		InitPerfTest(argv);
		
	InitWorkThread();
	scanf("%s", s);
	DestroyWorkThread();
	sleep(1);
	return 0;
}
