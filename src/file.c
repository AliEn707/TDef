#include "grid.h"
#include "file.h"
#include "gridmath.h"
#include "areaarray.h"
#include "engine.h"
#include "engine_tower.h"
#include "engine_npc.h"
#include "engine_bullet.h"
#include "types.h"

static signed err;

//map file parser
gnode * loadMap(char *path){
	gnode * grid;
	FILE * file;
//	int err;
	char filepath[25];
	sprintf(filepath,"../maps/%s.mp",path);
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
//		printDebug("%s\n",buf);
		if (strcmp(buf,"max_npcs")==0){
			int t_t;
			err=fscanf(file,"%d\n",&t_t);
			setNpcsMax(t_t);
			continue;
		}
		if (strcmp(buf,"max_towers")==0){
			int t_t;
			err=fscanf(file,"%d\n",&t_t);
			setTowersMax(t_t);
			continue;
		}
		if (strcmp(buf,"max_bullets")==0){
			int t_t;
			err=fscanf(file,"%d\n",&t_t);
			setBulletsMax(t_t);
			continue;
		}
		if (strcmp(buf,"pc_base")==0){
			//create base of pc player
			int base;
			int base_health;
			err=fscanf(file,"%d %d\n",&base,&base_health);
			setupPlayer(PC,ENEMY);
			config.players[PC].base_type.health=base_health;
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
			
			config.game.players=config.bases_size; //0 is AI and not playble
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
//	int err;
	char buf[100];
	printDebug("Load npc types....");
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
		err=fscanf(file,"%s ",buf);
//		printDebug("%s  ||\n",buf);
		if (strcmp(buf,"//-")==0){
			err=fscanf(file,"%s\n",buf);
			typesNpcAdd(n_n->id,n_n);
			n_n=0;
			continue;
		}
		if (strcmp(buf,"name")==0){
			err=fscanf(file,"%s\n",buf);
			continue;
		}
		if (strcmp(buf,"id")==0){
			err=fscanf(file,"%d\n",&n_n->id);
			continue;
		}
		if (strcmp(buf,"health")==0){
			err=fscanf(file,"%d\n",&n_n->health);
			continue;
		}
		if (strcmp(buf,"damage")==0){
			err=fscanf(file,"%d\n",&n_n->damage);
			continue;
		}
		if (strcmp(buf,"shield")==0){
			err=fscanf(file,"%d\n",&n_n->shield);
			continue;
		}
		if (strcmp(buf,"energy")==0){
			err=fscanf(file,"%d\n",&n_n->energy);
			continue;
		}
		if (strcmp(buf,"armor")==0){
			err=fscanf(file,"%f\n",&n_n->armor);
			continue;
		}		
		if (strcmp(buf,"support")==0){
			err=fscanf(file,"%d\n",&n_n->support);
			continue;
		}
		if (strcmp(buf,"see_distanse")==0){
			err=fscanf(file,"%d\n",&n_n->see_distanse);
			continue;
		}
		if (strcmp(buf,"attack_distanse")==0){
			err=fscanf(file,"%d\n",&n_n->attack_distanse);
			continue;
		}
		if (strcmp(buf,"attack_speed")==0){
			float tmp;
			err=fscanf(file,"%f\n",&tmp);
			n_n->attack_speed=TPS/tmp;
			continue;
		}
		if (strcmp(buf,"move_speed")==0){
			float tmp;
			err=fscanf(file,"%f\n",&tmp);
			n_n->move_speed=tmp/TPS;
			continue;
		}
		if (strcmp(buf,"cost")==0){
			err=fscanf(file,"%d\n",&n_n->cost);
			continue;
		}
		if (strcmp(buf,"receive")==0){
			err=fscanf(file,"%d\n",&n_n->receive);
			continue;
		}
		if (strcmp(buf,"bullet_type")==0){
			err=fscanf(file,"%d\n",&n_n->bullet_type);
			continue;
		}
		if (strcmp(buf,"type")==0){
			err=fscanf(file,"%d\n",&n_n->type);
			continue;
		}
		if (strcmp(buf,"attack_tower")==0){
			err=fscanf(file,"%hd\n",&n_n->attack_tower);
			continue;
		}		
	}
	
	if (n_n!=0)
		free(n_n);
	
	fclose(file);
	printDebug("done\n");
	return 0;
}

int loadTowerTypes(){
	char * filepath="../data/types/tower.cfg";
	FILE * file;
	tower_type* t_t=0;
	char buf[100];
//	int err;
	printDebug("Load tower types....");
	if ((file=fopen(filepath,"rt"))==0){
		perror("fopen loadTowerTypes");
		return 1;
	}
	
	while(feof(file)==0){
		memset(buf,0,sizeof(buf));
		err=fscanf(file,"%s ",buf);
//		printDebug("%s  ||\n",buf);
		if (t_t==0)
			if((t_t=malloc(sizeof(tower_type)))==0)
				perror("malloc tower loadTypes");
			
		if (strcmp(buf,"//-")==0){
			err=fscanf(file,"%s\n",buf);
			typesTowerAdd(t_t->id,t_t);
			t_t=0;
			continue;
		}
		if (strcmp(buf,"name")==0){
			err=fscanf(file,"%s\n",buf);
			continue;
		}
		if (strcmp(buf,"id")==0){
			err=fscanf(file,"%d\n",&t_t->id);
			continue;
		}
		if (strcmp(buf,"health")==0){
			err=fscanf(file,"%d\n",&t_t->health);
			continue;
		}
		if (strcmp(buf,"damage")==0){
			err=fscanf(file,"%d\n",&t_t->damage);
			continue;
		}
		if (strcmp(buf,"energy")==0){
			err=fscanf(file,"%d\n",&t_t->energy);
			continue;
		}
		if (strcmp(buf,"shield")==0){
			err=fscanf(file,"%d\n",&t_t->shield);
			continue;
		}
		if (strcmp(buf,"armor")==0){
			err=fscanf(file,"%f\n",&t_t->armor);
			continue;
		}			
		if (strcmp(buf,"attack_distanse")==0){
			err=fscanf(file,"%d\n",&t_t->distanse);
			continue;
		}
		if (strcmp(buf,"attack_speed")==0){
			float tmp;
			err=fscanf(file,"%f\n",&tmp);
			t_t->attack_speed=TPS/tmp;
			continue;
		}
		if (strcmp(buf,"cost")==0){
			err=fscanf(file,"%d\n",&t_t->cost);
			continue;
		}
		if (strcmp(buf,"receive")==0){
			err=fscanf(file,"%d\n",&t_t->receive);
			continue;
		}
		if (strcmp(buf,"ignor_type")==0){
			err=fscanf(file,"%d\n",&t_t->ignor_type);
			continue;
		}
		if (strcmp(buf,"prior_type")==0){
			err=fscanf(file,"%d\n",&t_t->prior_type);
			continue;
		}
		if (strcmp(buf,"bullet_type")==0){
			err=fscanf(file,"%d\n",&t_t->bullet_type);
			continue;
		}
		
	}
	
	if (t_t!=0)
		free(t_t);
	
	fclose(file);
	printDebug("done\n");
	return 0;
}

int loadBulletTypes(){
	char * filepath="../data/types/bullet.cfg";
	FILE * file;
	bullet_type* b_b=0;
	char buf[100];
//	int err;
	printDebug("Load bullet types....");
	if ((file=fopen(filepath,"rt"))==0){
		perror("fopen loadBulletTypes");
		return 1;
	}
	
	while(feof(file)==0){
		memset(buf,0,sizeof(buf));
		err=fscanf(file,"%s ",buf);
		if (b_b==0)
			if((b_b=malloc(sizeof(bullet_type)))==0)
				perror("malloc bullet loadTypes");
			
//		printDebug("%s  ||\n",buf);
		if (strcmp(buf,"name")==0){
			err=fscanf(file,"%s\n",buf);
			continue;
		}
		if (strcmp(buf,"//-")==0){
			err=fscanf(file,"%s\n",buf);
			typesBulletAdd(b_b->id,b_b);
			b_b=0;
			continue;
		}
		if (strcmp(buf,"id")==0){
			err=fscanf(file,"%d\n",&b_b->id);
			continue;
		}
		if (strcmp(buf,"speed")==0){
			float tmp;
			err=fscanf(file,"%f\n",&tmp);
			b_b->speed=tmp/TPS;
			continue;
		}
		if (strcmp(buf,"attack_type")==0){
			err=fscanf(file,"%d\n",&b_b->attack_type);
			continue;
		}
		if (strcmp(buf,"move_type")==0){
			err=fscanf(file,"%d\n",&b_b->move_type);
			continue;
		}
		
	}
	
	if (b_b!=0)
		free(b_b);
	
	fclose(file);
	printDebug("done\n");
	return 0;
}



void realizeTypes(){
//	free(config.bullet_types);
//	free(config.tower_types);
//	free(config.npc_types);
	typesNpcClear();
	typesTowerClear();
	typesBulletClear();
}

