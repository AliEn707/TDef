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
		/////////////////
		fscanf(file,"%d\n",&c);
		grid[i].buildable=(char)c;
		/////////////////
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
	FILE * file;
//	printf("loading configuration\n");
	if ((file=fopen(filepath,"r"))==0) 
		perror("fopen loadTypes\n");
	char buf[100];
	int i;
	fscanf(file,"%d ",&i);
	fscanf(file,"%s\n",buf);
	{
		tower_type t[1000];
		do{
			config.tower_types_size=i;
			fscanf(file,"%d ",&i);
			if (i==-1)
				fscanf(file,"%s\n",buf);
			else{
				t[i].id=i;
				fscanf(file,"%d %d %d %d %d %d\n",
						&t[i].health,
						&t[i].damage,
						&t[i].energy,
						&t[i].shield,
						&t[i].attack_speed,
						&t[i].cost
				);
				fscanf(file," %f %f %f %d %d\n",
						&t[i].effects.speed,
						&t[i].effects.shield,
						&t[i].effects.damage,
						&t[i].effects.time,
						&t[i].effects.status
				);
			}
		}while(i!=-1);
		if ((config.tower_types=malloc(sizeof(tower_type)*config.tower_types_size))==0)
			perror("malloc tower_types loadTypes\n");
		memcpy(config.tower_types,t,sizeof(tower_type)*config.tower_types_size);
	}	
	{
		npc_type t[1000];
		do{
			config.npc_types_size=i;
			fscanf(file,"%d ",&i);
			if (i==-1)
				fscanf(file,"%s\n",buf);
			else{
				t[i].id=i;
				fscanf(file,"%d %d %d %d %f %d\n",
						&t[i].health,
						&t[i].damage,
						&t[i].shield,
						&t[i].attack_speed,
						&t[i].move_speed,
						&t[i].cost
				);
				fscanf(file," %f %f %f %d %d\n",
						&t[i].effects.speed,
						&t[i].effects.shield,
						&t[i].effects.damage,
						&t[i].effects.time,
						&t[i].effects.status
				);
			}
		}while(i!=-1);
		if ((config.npc_types=malloc(sizeof(npc_type)*config.npc_types_size))==0)
			perror("malloc npc_types loadTypes\n");
		memcpy(config.npc_types,t,sizeof(npc_type)*config.npc_types_size);
	}	
//	printf("%d %d\n",config.tower_types_size,config.npc_types_size);
	fclose(file);
}


void realizeTypes(){
}

