#include "grid.h"
#include "file.h"
#include "gridmath.h"


gnode * loadMap(char *filepath){
	gnode * grid;
	FILE * file;
	if ((file=fopen(filepath,"r"))==0) 
		perror("fopen loadMap\n");
	int size;
	fscanf(file,"%d\n",&size);
	setGridSize(size);
	if ((grid=malloc(sizeof(gnode)*size*size))==0)
		perror("malloc loadMap\n");
	memset(grid,0,sizeof(gnode)*size*size);
	int i;
	for(i=0;i<size*size;i++){
		int c;
		fscanf(file,"%d\n",&c);
		grid[i].buildable=(char)c;
		grid[i].id=i;
	}
	
	fclose(file);
	return grid;
}

void realizeMap(gnode* grid){
	free(grid);
	
}

void loadConfig(char* filepath){
	FILE * file;
//	printf("loading configuration\n");
	if ((file=fopen(filepath,"r"))==0) 
		perror("fopen loadConfig\n");
	char buf[100];
	fscanf(file,"%d %s\n",config.tower_max,buf);
	fscanf(file,"%d %s\n",config.npc_max,buf);
	fscanf(file,"%d %s\n",config.bullet_max,buf);
	
	fclose(file);
}


void loadTypes(char * filepath){
	
	
}


void realizeTypes(){
}

