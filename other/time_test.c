#include <time.h>
#include <sys/time.h>
int main(){
	struct timeval start, end;
gettimeofday(&start, NULL);

// benchmark code

	while(1){
		sleep(1);
		gettimeofday(&end, NULL);

		double delta = ((end.tv_sec  - start.tv_sec) * 1000000u + 
		end.tv_usec - start.tv_usec) / 1.e6;
		int d=(end.tv_usec - start.tv_usec);
		float fd=(end.tv_usec - start.tv_usec)/ 1.e6;
		printf("%llg\n",delta);
		printf("%d\n",d);
		printf("%g\n",fd);
		
	//	printf("%llg\n\n",omp_get_wtime());
	
	}
	
}



int timePassed(){
	//config.time  struct timeval
	struct timeval end;
	gettimeofday(&end, NULL);
	int out=(end.tv_usec - config.time.tv_usec);
	memcpy(&config.time,&end,sizeof(end));
	return out;
}

*/
