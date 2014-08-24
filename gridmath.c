#include <math.h>
#include <stdio.h>
#include "grid.h"



float cos_grid[6284];
float sin_grid[6284];
//float sqrt_grid[20001];

float gcos(float a){
	int angle=(int)(a*1000);
	if (angle<0) angle*=-1;
	if (angle>6283) angle%=6284;
	return cos_grid[angle];
}

float gsin(float a){
	int i=1;
	int angle=(int)(a*1000);
	if (angle<0) i*=-1;
	if (angle>6283) angle%=6284;
	return sin_grid[angle]*i;
}

//float gsqrt(float a){
//	if (a<0) return 0;
//	if (a>=20)return sqrt(a);
//	return sqrt_grid[(int)(a*1000)];
//}

void initGridMath(){
	float i;
	for(i=0;i<6.284;i+=0.001){
		sin_grid[(int)(i*1000)]=sinf(i);
		cos_grid[(int)(i*1000)]=cosf(i);
	}
//	for(i=0;i<=20;i+=0.001)
//		sqrt_grid[(int)(i*1000)]=sqrt(i);
		
}

float glength(vec* v1,vec* v2){
	///need to fix it
	//char  buf[15];
	//sprintf(buf,"|%g\n",sqr(v1->x-v2->x)+sqr(v1->y-v2->y));
	///////////
	return sqrt(sqr(v1->x-v2->x)+sqr(v1->y-v2->y));
}

float getDir(vec* v1,vec* v2, vec* out){
	float length=glength(v1,v2);
//	printf("%g\n",length);
	out->x=(v2->x-v1->x)/length;
	out->y=(v2->y-v1->y)/length;
	return length;
}
//test
/*
main(){
	float i;
	initGridFunc();
	for(i=0;i<6.284;i+=0.01){
		printf("%g %g  %g %g\n",sin(i),gsin(i),cos(i),gcos(i));
	}
	for(i=0;i<=500;i+=0.015)
		printf("%g %g\n",sqrt(i),gsqrt(i));
	printf("%g %g",sqrt(45),gsqrt(45));
	sleep(10);
}
*/