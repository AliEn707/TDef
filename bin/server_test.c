#include <stdlib.h>
#include <stdio.h>
#include <time.h>

const char filename[] = "log.txt";

int main (int argc, char **argv) {
	time_t rawtime;
	struct tm * timeinfo;
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	
	FILE *f = fopen(filename, "a");
	int i;
	fprintf(f, "Now: %s %s started with args: ", asctime(timeinfo), argv[0]);
	for (i = 1; i < argc; i++)
		fprintf(f, "%s ", argv[i]);
	fprintf(f, "\n");
	fclose(f);
	return 0;
}
