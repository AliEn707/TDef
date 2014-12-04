#include "grid.h"
#include "file.h"
#include "gridmath.h"
#include "areaarray.h"
#include "engine.h"
#include "engine_tower.h"
#include "engine_npc.h"
#include "engine_bullet.h"
#include "types.h"

//map file parser
gnode * loadMap(char *path){
	gnode * grid;
	FILE * file;
	int err;
	char filepath[25];
	sprintf(filepath,"../%s.mp",path);
	config.global_id=1;
	memset(config.players,0,sizeof(player)*PLAYER_MAX);
	initAreaArray();
	
	if ((file=fopen(filepath,"r"))==0) 
		perror("fopen loadMap");
	char buf[100];
	int size;
	err=fscanf(file,"%d\n",&size);
	if (err<=0)
		perror("fscanf loadMap");
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
	err=fscanf(file,"%s\n",walk);
	err=fscanf(file,"%s\n",build);
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
		err=fscanf(file,"%s ",buf);
//		printf("%s\n",buf);
		if (strcmp(buf,"max_npcs")==0){
			err=fscanf(file,"%d\n",&config.npc_max);
			if ((config.npc_array=malloc(sizeof(npc)*config.npc_max))==0)
				perror("malloc NPC initArrays");
			memset(config.npc_array,0,sizeof(npc)*config.npc_max);
			continue;
		}
		if (strcmp(buf,"max_towers")==0){
			err=fscanf(file,"%d\n",&config.tower_max);
			if ((config.tower_array=malloc(sizeof(tower)*config.tower_max))==0)
				perror("malloc tower initArrays");
			memset(config.tower_array,0,sizeof(tower)*config.tower_max);
			continue;
		}
		if (strcmp(buf,"max_bullets")==0){
			err=fscanf(file,"%d\n",&config.bullet_max);
			if ((config.bullet_array=malloc(sizeof(bullet)*config.bullet_max))==0)
				perror("malloc bullet initArrays");
			memset(config.bullet_array,0,sizeof(bullet)*config.bullet_max);
			continue;
		}
		if (strcmp(buf,"pc_base")==0){
			//create base of pc player
			int base;
			int base_health;
			err=fscanf(file,"%d %d\n",&base,&base_health);
			setupPlayer(PC,ENEMY,base_health);
			setPlayerBase(PC,spawnTower(grid,config.bases[base].position,PC,BASE));
			continue;
		}
		if (strcmp(buf,"bases")==0){
			int i;
			err=fscanf(file,"%d\n",&config.bases_size);
			if ((config.bases=malloc(config.bases_size*sizeof(base)))==0)
				perror("malloc config.bases loadMap");
			memset(config.bases,0,config.bases_size*sizeof(base));
			for(i=0;i<config.bases_size;i++){
				int j;
				err=fscanf(file,"%d ",&j);
				err=fscanf(file,"%d %d\n",&config.bases[j].position,&config.bases[j].point_id);
				config.bases[j].id=j;
			}
			continue;
		}
		if (strcmp(buf,"points")==0){
			int i;
			err=fscanf(file,"%d\n",&config.points_size);
			if ((config.points=malloc(config.points_size*sizeof(point)))==0)
				perror("malloc config.points loadMap");
			memset(config.points,0,config.points_size*sizeof(point));
			for(i=0;i<config.points_size;i++){
				int j;
				err=fscanf(file,"%d ",&j);
				err=fscanf(file,"%d\n",&config.points[j].position);
				config.points[j].id=j;
			}
			continue;
		}
		if (strcmp(buf,"waves")==0){
			int i;
			float f;
			err=fscanf(file,"%d \n",&config.waves_size);
			if((config.waves=malloc(config.waves_size*sizeof(wave)))==0)
				perror("malloc config.waves loadMap");
			memset(config.waves,0,config.waves_size*sizeof(wave));
			for(i=0;i<config.waves_size;i++){
				int j;
				err=fscanf(file,"%s %d %f\n",buf,&config.waves[i].parts_num,&f);
				config.waves[i].delay=f*TPS;
				if((config.waves[i].parts=malloc(config.waves[i].parts_num*sizeof(wave_part)))==0)
					perror("malloc config.waves[i].parts loadMap");
				memset(config.waves[i].parts,0,config.waves[i].parts_num*sizeof(wave_part));
				for(j=0;j<config.waves[i].parts_num;j++){
					err=fscanf(file,"%d %d %d %f\n",&config.waves[i].parts[j].point,
											&config.waves[i].parts[j].npc_type,
											&config.waves[i].parts[j].num,
											&f);
					config.waves[i].parts[j].delay=f*TPS;
				}
			}
			continue;
		}
		
	}
	fclose(file);
	return grid;
}


void realizeMap(gnode* grid){
	free(grid);
	
}

int loadNpcTypes(){
	char * filepath="../data/types/npc.cfg";
	FILE * file;
	npc_type* n_n=0;
	char buf[100];
	printf("Load npc types....");
	if ((file=fopen(filepath,"rt"))==0){
		perror("fopen loadNpcTypes");
		return 1;
	}
	
	while(feof(file)==0){
		if (n_n==0)
			if((n_n=malloc(sizeof(npc_type)))==0){
				perror("malloc npc loadTypes");
				return 1;
			}
		memset(buf,0,sizeof(buf));
		fscanf(file,"%s ",buf);
//		printf("%s  ||\n",buf);
		if (strcmp(buf,"//-")==0){
			fscanf(file,"%s\n",buf);
			typesNpcAdd(n_n->id,n_n);
			n_n=0;
			continue;
		}
		if (strcmp(buf,"name")==0){
			fscanf(file,"%s\n",buf);
			continue;
		}
		if (strcmp(buf,"id")==0){
			fscanf(file,"%d\n",&n_n->id);
			continue;
		}
		if (strcmp(buf,"health")==0){
			fscanf(file,"%d\n",&n_n->health);
			continue;
		}
		if (strcmp(buf,"damage")==0){
			fscanf(file,"%d\n",&n_n->damage);
			continue;
		}
		if (strcmp(buf,"shield")==0){
			fscanf(file,"%d\n",&n_n->shield);
			continue;
		}
		if (strcmp(buf,"support")==0){
			fscanf(file,"%d\n",&n_n->support);
			continue;
		}
		if (strcmp(buf,"see_distanse")==0){
			fscanf(file,"%d\n",&n_n->see_distanse);
			continue;
		}
		if (strcmp(buf,"attack_distanse")==0){
			fscanf(file,"%d\n",&n_n->attack_distanse);
			continue;
		}
		if (strcmp(buf,"attack_speed")==0){
			float tmp;
			fscanf(file,"%f\n",&tmp);
			n_n->attack_speed=TPS/tmp;
			continue;
		}
		if (strcmp(buf,"move_speed")==0){
			float tmp;
			fscanf(file,"%f\n",&tmp);
			n_n->move_speed=tmp/TPS;
			continue;
		}
		if (strcmp(buf,"cost")==0){
			fscanf(file,"%d\n",&n_n->cost);
			continue;
		}
		if (strcmp(buf,"receive")==0){
			fscanf(file,"%d\n",&n_n->receive);
			continue;
		}
		if (strcmp(buf,"bullet_type")==0){
			fscanf(file,"%d\n",&n_n->bullet_type);
			continue;
		}
		if (strcmp(buf,"type")==0){
			fscanf(file,"%d\n",&n_n->type);
			continue;
		}
		
	}
	
	if (n_n!=0)
		free(n_n);
	
	fclose(file);
	printf("done\n");
	return 0;
}

int loadTowerTypes(){
	char * filepath="../data/types/tower.cfg";
	FILE * file;
	tower_type* t_t=0;
	char buf[100];
	printf("Load tower types....");
	if ((file=fopen(filepath,"rt"))==0){
		perror("fopen loadTowerTypes");
		return 1;
	}
	
	while(feof(file)==0){
		memset(buf,0,sizeof(buf));
		fscanf(file,"%s ",buf);
//		printf("%s  ||\n",buf);
		if (t_t==0)
			if((t_t=malloc(sizeof(tower_type)))==0)
				perror("malloc tower loadTypes");
			
		if (strcmp(buf,"//-")==0){
			fscanf(file,"%s\n",buf);
			typesTowerAdd(t_t->id,t_t);
			t_t=0;
			continue;
		}
		if (strcmp(buf,"name")==0){
			fscanf(file,"%s\n",buf);
			continue;
		}
		if (strcmp(buf,"id")==0){
			fscanf(file,"%d\n",&t_t->id);
			continue;
		}
		if (strcmp(buf,"health")==0){
			fscanf(file,"%d\n",&t_t->health);
			continue;
		}
		if (strcmp(buf,"damage")==0){
			fscanf(file,"%d\n",&t_t->damage);
			continue;
		}
		if (strcmp(buf,"energy")==0){
			fscanf(file,"%d\n",&t_t->energy);
			continue;
		}
		if (strcmp(buf,"shield")==0){
			fscanf(file,"%d\n",&t_t->shield);
			continue;
		}
		if (strcmp(buf,"attack_distanse")==0){
			fscanf(file,"%d\n",&t_t->distanse);
			continue;
		}
		if (strcmp(buf,"attack_speed")==0){
			float tmp;
			fscanf(file,"%f\n",&tmp);
			t_t->attack_speed=TPS/tmp;
			continue;
		}
		if (strcmp(buf,"cost")==0){
			fscanf(file,"%d\n",&t_t->cost);
			continue;
		}
		if (strcmp(buf,"ignor_type")==0){
			fscanf(file,"%d\n",&t_t->ignor_type);
			continue;
		}
		if (strcmp(buf,"prior_type")==0){
			fscanf(file,"%d\n",&t_t->prior_type);
			continue;
		}
		if (strcmp(buf,"bullet_type")==0){
			fscanf(file,"%d\n",&t_t->bullet_type);
			continue;
		}
		
	}
	
	if (t_t!=0)
		free(t_t);
	
	fclose(file);
	printf("done\n");
	return 0;
}

int loadBulletTypes(){
	char * filepath="../data/types/bullet.cfg";
	FILE * file;
	bullet_type* b_b=0;
	char buf[100];
	printf("Load bullet types....");
	if ((file=fopen(filepath,"rt"))==0){
		perror("fopen loadBulletTypes");
		return 1;
	}
	
	while(feof(file)==0){
		memset(buf,0,sizeof(buf));
		fscanf(file,"%s ",buf);
		if (b_b==0)
			if((b_b=malloc(sizeof(bullet_type)))==0)
				perror("malloc bullet loadTypes");
			
//		printf("%s  ||\n",buf);
		if (strcmp(buf,"name")==0){
			fscanf(file,"%s\n",buf);
			continue;
		}
		if (strcmp(buf,"//-")==0){
			fscanf(file,"%s\n",buf);
			typesBulletAdd(b_b->id,b_b);
			b_b=0;
			continue;
		}
		if (strcmp(buf,"id")==0){
			fscanf(file,"%d\n",&b_b->id);
			continue;
		}
		if (strcmp(buf,"speed")==0){
			float tmp;
			fscanf(file,"%f\n",&tmp);
			b_b->speed=tmp/TPS;
			continue;
		}
		if (strcmp(buf,"attack_type")==0){
			fscanf(file,"%d\n",&b_b->attack_type);
			continue;
		}
		if (strcmp(buf,"move_type")==0){
			fscanf(file,"%d\n",&b_b->move_type);
			continue;
		}
		
	}
	
	if (b_b!=0)
		free(b_b);
	
	fclose(file);
	printf("done\n");
	return 0;
}


/*
void loadTypes(char * filepath){
	FILE * file;
	int err;
//	printf("loading configuration\n");
	if ((file=fopen(filepath,"r"))==0) 
		perror("fopen loadTypes");
	char buf[100];
	int i=1;
	while(feof(file)==0){
		memset(buf,0,sizeof(buf));
		err=fscanf(file,"%s ",buf);
		if (err<=0)
		perror("fscanf loadTypes");
//		printf("%s  ||\n",buf);
		if (strcmp(buf,"TOWER_TYPE")==0){
			int tmp;
			err=fscanf(file,"%d\n",&tmp);
			if((config.tower_types=malloc(sizeof(tower_type)*(tmp+1)))==0)
				perror("malloc tower loadTypes");
			continue;
		}
		if (strcmp(buf,"NPC_TYPE")==0){
			int tmp;
			err=fscanf(file,"%d\n",&tmp);
			if((config.npc_types=malloc(sizeof(npc_type)*(tmp+1)))==0)
				perror("malloc npc loadTypes");
			break;
		}
		if (strcmp(buf,"//-")==0){
			err=fscanf(file,"%s\n",buf);
			i++;
			continue;
		}
		if (strcmp(buf,"name")==0){
			err=fscanf(file,"%s\n",buf);
			continue;
		}
		if (strcmp(buf,"id")==0){
			err=fscanf(file,"%d\n",&config.tower_types[i].id);
			continue;
		}
		if (strcmp(buf,"health")==0){
			err=fscanf(file,"%d\n",&config.tower_types[i].health);
			continue;
		}
		if (strcmp(buf,"damage")==0){
			err=fscanf(file,"%d\n",&config.tower_types[i].damage);
			continue;
		}
		if (strcmp(buf,"energy")==0){
			err=fscanf(file,"%d\n",&config.tower_types[i].energy);
			continue;
		}
		if (strcmp(buf,"shield")==0){
			err=fscanf(file,"%d\n",&config.tower_types[i].shield);
			continue;
		}
		if (strcmp(buf,"attack_distanse")==0){
			err=fscanf(file,"%d\n",&config.tower_types[i].distanse);
			continue;
		}
		if (strcmp(buf,"attack_speed")==0){
			float tmp;
			err=fscanf(file,"%f\n",&tmp);
			config.tower_types[i].attack_speed=TPS/tmp;
			continue;
		}
		if (strcmp(buf,"cost")==0){
			err=fscanf(file,"%d\n",&config.tower_types[i].cost);
			continue;
		}
		if (strcmp(buf,"ignor_type")==0){
			err=fscanf(file,"%d\n",&config.tower_types[i].ignor_type);
			continue;
		}
		if (strcmp(buf,"prior_type")==0){
			err=fscanf(file,"%d\n",&config.tower_types[i].prior_type);
			continue;
		}
		if (strcmp(buf,"bullet_type")==0){
			err=fscanf(file,"%d\n",&config.tower_types[i].bullet_type);
			continue;
		}
		
	}
	config.tower_types_size=i;
	printf("\t\t%d\n",config.tower_types_size);
	i=1;
	while(feof(file)==0){
		memset(buf,0,sizeof(buf));
		err=fscanf(file,"%s ",buf);
//		printf("%s  ||\n",buf);
		if (strcmp(buf,"BULLET_TYPE")==0){
			int tmp;
			err=fscanf(file,"%d\n",&tmp);
			if((config.bullet_types=malloc(sizeof(bullet_type)*(tmp+1)))==0)
				perror("malloc tower loadTypes");
			break;
		}
		if (strcmp(buf,"//-")==0){
			err=fscanf(file,"%s\n",buf);
			i++;
//			config.npc_types[i].attack_tower=1;
			continue;
		}
		if (strcmp(buf,"name")==0){
			err=fscanf(file,"%s\n",buf);
			continue;
		}
		if (strcmp(buf,"id")==0){
			err=fscanf(file,"%d\n",&config.npc_types[i].id);
			continue;
		}
		if (strcmp(buf,"health")==0){
			err=fscanf(file,"%d\n",&config.npc_types[i].health);
			continue;
		}
		if (strcmp(buf,"damage")==0){
			err=fscanf(file,"%d\n",&config.npc_types[i].damage);
			continue;
		}
		if (strcmp(buf,"shield")==0){
			err=fscanf(file,"%d\n",&config.npc_types[i].shield);
			continue;
		}
		if (strcmp(buf,"support")==0){
			err=fscanf(file,"%d\n",&config.npc_types[i].support);
			continue;
		}
		if (strcmp(buf,"see_distanse")==0){
			err=fscanf(file,"%d\n",&config.npc_types[i].see_distanse);
			continue;
		}
		if (strcmp(buf,"attack_distanse")==0){
			err=fscanf(file,"%d\n",&config.npc_types[i].attack_distanse);
			continue;
		}
		if (strcmp(buf,"attack_speed")==0){
			float tmp;
			err=fscanf(file,"%f\n",&tmp);
			config.npc_types[i].attack_speed=TPS/tmp;
			continue;
		}
		if (strcmp(buf,"move_speed")==0){
			float tmp;
			err=fscanf(file,"%f\n",&tmp);
			config.npc_types[i].move_speed=tmp/TPS;
			continue;
		}
		if (strcmp(buf,"cost")==0){
			err=fscanf(file,"%d\n",&config.npc_types[i].cost);
			continue;
		}
		if (strcmp(buf,"receive")==0){
			err=fscanf(file,"%d\n",&config.npc_types[i].receive);
			continue;
		}
		if (strcmp(buf,"bullet_type")==0){
			err=fscanf(file,"%d\n",&config.npc_types[i].bullet_type);
			continue;
		}
		if (strcmp(buf,"type")==0){
			err=fscanf(file,"%d\n",&config.npc_types[i].type);
			continue;
		}
		
	}
	config.npc_types_size=i;
	i=1;
	while(feof(file)==0){
		memset(buf,0,sizeof(buf));
		err=fscanf(file,"%s ",buf);
//		printf("%s  ||\n",buf);
		if (strcmp(buf,"name")==0){
			err=fscanf(file,"%s\n",buf);
			continue;
		}
		if (strcmp(buf,"//-")==0){
			err=fscanf(file,"%s\n",buf);
			i++;
			continue;
		}
		if (strcmp(buf,"id")==0){
			err=fscanf(file,"%d\n",&config.bullet_types[i].id);
			continue;
		}
		if (strcmp(buf,"speed")==0){
			float tmp;
			err=fscanf(file,"%f\n",&tmp);
			config.bullet_types[i].speed=tmp/TPS;
			continue;
		}
		if (strcmp(buf,"attack_type")==0){
			err=fscanf(file,"%d\n",&config.bullet_types[i].attack_type);
			continue;
		}
		if (strcmp(buf,"move_type")==0){
			err=fscanf(file,"%d\n",&config.bullet_types[i].move_type);
			continue;
		}
		
	}
	config.bullet_types_size=i;
//	printf("%d %d\n",config.tower_types_size,config.npc_types_size);
	fclose(file);
}
*/

void realizeTypes(){
//	free(config.bullet_types);
//	free(config.tower_types);
//	free(config.npc_types);
	typesNpcClear();
	typesTowerClear();
	typesBulletClear();
}

