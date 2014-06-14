#include "grid.h"
#include "file.h"
#include "gridmath.h"


gnode * loadMap(char *filepath){
	gnode * grid;
	FILE * file;
	if ((file=fopen(filepath,"r"))==0) 
		perror("fopen loadMap");
	char buf[100];
	int size;
	fscanf(file,"%d\n",&size);
	setGridSize(size);
	if ((grid=malloc(sizeof(gnode)*size*size))==0)
		perror("malloc grid loadMap");
	memset(grid,0,sizeof(gnode)*size*size);
	char* walk;
	char* build;
	if((walk=malloc((size*size+1)*sizeof(char)))==0)
		perror("malloc walk loadMap");
	if((build=malloc((size*size+1)*sizeof(char)))==0)
		perror("malloc build loadMap");
	fscanf(file,"%s\n",walk);
	fscanf(file,"%s\n",build);
	int i;
	for(i=0;i<size*size;i++){
		grid[i].id=i;
		grid[i].walkable= walk[i]=='1'?(char)1:walk[i]=='0'?(char)0:(char)-1;
		grid[i].buildable= build[i]=='1'?(char)1:build[i]=='0'?(char)0:(char)-1;
	}
	free(walk);
	free(build);
	while(feof(file)==0){
		memset(buf,0,sizeof(buf));
		fscanf(file,"%s ",buf);
		printf("%s\n",buf);
		if (strcmp(buf,"max_npcs")==0){
			fscanf(file,"%d\n",&config.npc_max);
			continue;
		}
		if (strcmp(buf,"max_towers")==0){
			fscanf(file,"%d\n",&config.tower_max);
			printf("%d\n",config.tower_max);
			continue;
		}
		if (strcmp(buf,"max_bullets")==0){
			fscanf(file,"%d\n",&config.bullet_max);
			printf("%d\n",config.bullet_max);
			continue;
		}
		
	}
	fclose(file);
	config.global_id=1;
	return grid;
}


void realizeMap(gnode* grid){
	free(grid);
	
}


void loadTypes(char * filepath){
	FILE * file;
//	printf("loading configuration\n");
	if ((file=fopen(filepath,"r"))==0) 
		perror("fopen loadTypes");
	char buf[100];
	int i=1;
	while(feof(file)==0){
		memset(buf,0,sizeof(buf));
		fscanf(file,"%s ",buf);
//		printf("%s  ||\n",buf);
		if (strcmp(buf,"TOWER_TYPE")==0){
			int tmp;
			fscanf(file,"%d\n",&tmp);
			if((config.tower_types=malloc(sizeof(tower_type)*(tmp+1)))==0)
				perror("malloc tower loadTypes");
			continue;
		}
		if (strcmp(buf,"NPC_TYPE")==0){
			int tmp;
			fscanf(file,"%d\n",&tmp);
			if((config.npc_types=malloc(sizeof(npc_type)*(tmp+1)))==0)
				perror("malloc npc loadTypes");
			break;
		}
		if (strcmp(buf,"//-")==0){
			fscanf(file,"%s\n",buf);
			i++;
			continue;
		}
		if (strcmp(buf,"name")==0){
			fscanf(file,"%s\n",buf);
			continue;
		}
		if (strcmp(buf,"id")==0){
			fscanf(file,"%d\n",&config.tower_types[i].id);
			continue;
		}
		if (strcmp(buf,"health")==0){
			fscanf(file,"%d\n",&config.tower_types[i].health);
			continue;
		}
		if (strcmp(buf,"damage")==0){
			fscanf(file,"%d\n",&config.tower_types[i].damage);
			continue;
		}
		if (strcmp(buf,"energy")==0){
			fscanf(file,"%d\n",&config.tower_types[i].energy);
			continue;
		}
		if (strcmp(buf,"shield")==0){
			fscanf(file,"%d\n",&config.tower_types[i].shield);
			continue;
		}
		if (strcmp(buf,"attack_distanse")==0){
			fscanf(file,"%d\n",&config.tower_types[i].distanse);
			continue;
		}
		if (strcmp(buf,"attack_speed")==0){
			float tmp;
			fscanf(file,"%f\n",&tmp);
			config.tower_types[i].attack_speed=TPS/tmp;
			continue;
		}
		if (strcmp(buf,"cost")==0){
			fscanf(file,"%d\n",&config.tower_types[i].cost);
			continue;
		}
		if (strcmp(buf,"ignor_type")==0){
			fscanf(file,"%d\n",&config.tower_types[i].ignor_type);
			continue;
		}
		if (strcmp(buf,"prior_type")==0){
			fscanf(file,"%d\n",&config.tower_types[i].prior_type);
			continue;
		}
		if (strcmp(buf,"bullet_type")==0){
			fscanf(file,"%d\n",&config.tower_types[i].bullet_type);
			continue;
		}
		
	}
	config.tower_types_size=i;
	printf("\t\t%d\n",config.tower_types_size);
	i=1;
	while(feof(file)==0){
		memset(buf,0,sizeof(buf));
		fscanf(file,"%s ",buf);
//		printf("%s  ||\n",buf);
		if (strcmp(buf,"BULLET_TYPE")==0){
			int tmp;
			fscanf(file,"%d\n",&tmp);
			if((config.bullet_types=malloc(sizeof(bullet_type)*(tmp+1)))==0)
				perror("malloc tower loadTypes");
			break;
		}
		if (strcmp(buf,"//-")==0){
			fscanf(file,"%s\n",buf);
			i++;
			continue;
		}
		if (strcmp(buf,"name")==0){
			fscanf(file,"%s\n",buf);
			continue;
		}
		if (strcmp(buf,"id")==0){
			fscanf(file,"%d\n",&config.npc_types[i].id);
			continue;
		}
		if (strcmp(buf,"health")==0){
			fscanf(file,"%d\n",&config.npc_types[i].health);
			continue;
		}
		if (strcmp(buf,"damage")==0){
			fscanf(file,"%d\n",&config.npc_types[i].damage);
			continue;
		}
		if (strcmp(buf,"shield")==0){
			fscanf(file,"%d\n",&config.npc_types[i].shield);
			continue;
		}
		if (strcmp(buf,"support")==0){
			fscanf(file,"%d\n",&config.npc_types[i].support);
			continue;
		}
		if (strcmp(buf,"see_distanse")==0){
			fscanf(file,"%d\n",&config.npc_types[i].see_distanse);
			continue;
		}
		if (strcmp(buf,"attack_distanse")==0){
			fscanf(file,"%d\n",&config.npc_types[i].attack_distanse);
			continue;
		}
		if (strcmp(buf,"attack_speed")==0){
			float tmp;
			fscanf(file,"%f\n",&tmp);
			config.npc_types[i].attack_speed=TPS/tmp;
			continue;
		}
		if (strcmp(buf,"move_speed")==0){
			float tmp;
			fscanf(file,"%f\n",&tmp);
			config.npc_types[i].move_speed=tmp/TPS;
			continue;
		}
		if (strcmp(buf,"cost")==0){
			fscanf(file,"%d\n",&config.npc_types[i].cost);
			continue;
		}
		if (strcmp(buf,"receive")==0){
			fscanf(file,"%d\n",&config.npc_types[i].receive);
			continue;
		}
		if (strcmp(buf,"bullet_type")==0){
			fscanf(file,"%d\n",&config.npc_types[i].bullet_type);
			continue;
		}
		if (strcmp(buf,"type")==0){
			fscanf(file,"%d\n",&config.npc_types[i].type);
			continue;
		}
		
	}
	config.npc_types_size=i;
	i=1;
	while(feof(file)==0){
		memset(buf,0,sizeof(buf));
		fscanf(file,"%s ",buf);
//		printf("%s  ||\n",buf);
		if (strcmp(buf,"name")==0){
			fscanf(file,"%s\n",buf);
			continue;
		}
		if (strcmp(buf,"//-")==0){
			fscanf(file,"%s\n",buf);
			i++;
			continue;
		}
		if (strcmp(buf,"id")==0){
			fscanf(file,"%d\n",&config.bullet_types[i].id);
			continue;
		}
		if (strcmp(buf,"speed")==0){
			float tmp;
			fscanf(file,"%f\n",&tmp);
			config.bullet_types[i].speed=tmp/TPS;
			continue;
		}
		if (strcmp(buf,"attack_type")==0){
			fscanf(file,"%d\n",&config.bullet_types[i].attack_type);
			continue;
		}
		if (strcmp(buf,"move_type")==0){
			fscanf(file,"%d\n",&config.bullet_types[i].move_type);
			continue;
		}
		
	}
	config.bullet_types_size=i;
//	printf("%d %d\n",config.tower_types_size,config.npc_types_size);
	fclose(file);
}


void realizeTypes(){
	free(config.tower_types);
	free(config.npc_types);
	
}

