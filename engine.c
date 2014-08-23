#include "grid.h"
#include "engine.h"
#include "engine_npc.h"
#include "engine_tower.h"
#include "engine_bullet.h"
#include "areaarray.h"


int timePassed(int i){
	//config.time  struct timeval
	struct timeval end;
	gettimeofday(&end, NULL);
	int out=((end.tv_sec - config.time.tv_sec)*1000000+
			end.tv_usec - config.time.tv_usec);
	if(i>0)
		if (out>1000000/TPS)
			perror("time to tick");
	memcpy(&config.time,&end,sizeof(end));
	return out;
}


void syncTPS(){
	int z=timePassed(1);
	int must=1000;
	if((z=(1000000/TPS-must)-z)>0){
		usleep(z);
	}
	usleep(must);
	timePassed(0);
}


void initArrays(){
	if ((config.tower_array=malloc(sizeof(tower)*config.tower_max))==0)
		perror("malloc tower initArrays");
	memset(config.tower_array,0,sizeof(tower)*config.tower_max);
	if ((config.npc_array=malloc(sizeof(npc)*config.npc_max))==0)
		perror("malloc NPC initArrays");
	memset(config.npc_array,0,sizeof(npc)*config.npc_max);
	if ((config.bullet_array=malloc(sizeof(bullet)*config.bullet_max))==0)
		perror("malloc bullet initArrays");
	memset(config.bullet_array,0,sizeof(bullet)*config.bullet_max);
//	if ((config.players=malloc(sizeof(bullet)*config.player_max))==0)
//		perror("malloc player initArrays");
//	memset(config.players,0,sizeof(bullet)*config.player_max);
	memset(config.players,0,sizeof(bullet)*PLAYER_MAX);
	initAreaArray();
//	printf("%d %d %d\n",sizeof(tower)*config.tower_max,sizeof(npc)*config.npc_max,sizeof(bullet)*config.bullet_max);
}

void realizeArrays(){
//	free(config.players);
	free(config.tower_array);
	free(config.npc_array);
	free(config.bullet_array);
	free(config.points);
	free(config.bases);
	if (config.waves!=0){
		int i;
		for(i=0;i<config.waves_size;i++)
			free(config.waves[i].parts);
		free(config.waves);
	}
	realizeAreaArray();	
}

//helper 

int canSee(gnode* grid,vec* a,vec* b){
	float x1=a->x;
	float y1=a->y;
	float x2=b->x;
	float y2=b->y;
	int destination=to2d((int)x2,(int)y2);
//	printf("%g %g %g %g\n",x1,y1,x2,y2);
	if (x1!=x2){
		if (x1>x2){
			int tmp;
			tmp=x1;
			x1=x2;
			x2=tmp;
			tmp=y1;
			y1=y2;
			y2=tmp;
		}
	
		float K=(y2-y1)/(x2-x1);
		float B=(y1*(x2-x1)-x1*(y2-y1))/(x2-x1);
		 
		for(;x1<x2;x1+=0.3){
			y1=K*x1+B;
//			printf("1} %d\n",to2d(((int)x1),((int)y1)));
			if (to2d(((int)x1),((int)y1))!=destination){
				if (grid[to2d(((int)x1),((int)y1))].walkable<0||
					grid[to2d(((int)x1),((int)y1))].tower>0){
//					printf("!\n");
					return -1;
				}
			}
		}
	}else{
		if (y1>y2){
			int tmp;
			tmp=x1;
			x1=x2;
			x2=tmp;
			tmp=y1;
			y1=y2;
			y2=tmp;
		}
	
		float K=(x2-x1)/(y2-y1);
		float B=(x1*(y2-y1)-y1*(x2-x1))/(y2-y1);
		 
		for(;y1<y2;y1+=0.3){
			x1=K*y1+B;
//			printf("2} %d\n",to2d(((int)x1),((int)y1)));
			if (to2d(((int)x1),((int)y1))!=destination){
				if (grid[to2d(((int)x1),((int)y1))].walkable<0||
						grid[to2d(((int)x1),((int)y1))].tower>0)
					return -1;
			}
		}
	}
	return 1;
}

int canWalkThrough(gnode* grid,vec* a,vec* b){
	float x1=a->x;
	float y1=a->y;
	float x2=b->x;
	float y2=b->y;
	int destination=to2d((int)x2,(int)y2);
	if (x1!=x2){
		if (x1>x2){
			int tmp;
			tmp=x1;
			x1=x2;
			x2=tmp;
			tmp=y1;
			y1=y2;
			y2=tmp;
		}
		float K=(y2-y1)/(x2-x1);
		float B=(y1*(x2-x1)-x1*(y2-y1))/(x2-x1);
		
		for(;x1<=x2;x1+=0.3){
			y1=K*x1+B;
			if (to2d(((int)x1),((int)y1))!=destination){
				if (grid[to2d(((int)x1),((int)y1))].walkable<=0||
						grid[to2d(((int)x1),((int)y1))].tower>0)
					return -1;
			}
		}
	}else{
		if (y1>y2){
			int tmp;
			tmp=x1;
			x1=x2;
			x2=tmp;
			tmp=y1;
			y1=y2;
			y2=tmp;
		}
		float K=(x2-x1)/(y2-y1);
		float B=(x1*(y2-y1)-y1*(x2-x1))/(y2-y1);
		 
		for(;y1<=y2;y1+=0.3){
			x1=K*y1+B;
			if (to2d(((int)x1),((int)y1))!=destination){
				if (grid[to2d(((int)x1),((int)y1))].walkable<=0||
						grid[to2d(((int)x1),((int)y1))].tower>0)
					return -1;
			}
		}
	}
	return 1;
}

//scenari

//need more tests
void processWaves(gnode* grid){
	if (config.waves_size==0)
		return;
	if (config.wave_current.wave_num>=config.waves_size)
		return;
	config.wave_current.wave_ticks++;
	#define wave_curr config.waves[config.wave_current.wave_num]
	if(config.wave_current.wave_ticks>=wave_curr.delay){
		int check(){
			int i;
			for(i=0;i<wave_curr.parts_num;i++){
				if (wave_curr.parts[i].spawned<wave_curr.parts[i].num)
					return -1;
			}
			return 1;
		}
		int i;
		for(i=0;i<wave_curr.parts_num;i++)
			if(config.wave_current.wave_ticks%wave_curr.parts[i].delay==0)
				if (wave_curr.parts[i].spawned<wave_curr.parts[i].num){
					wave_curr.parts[i].spawned++;
					spawnNpc(grid,config.points[wave_curr.parts[i].point].position,ENEMY,wave_curr.parts[i].npc_type);
				}
		if (check()>0){
			config.wave_current.wave_ticks=0;
			config.wave_current.wave_num++;
		}
	}
}



//player 
void setupPlayer(int id,int isfriend,int base_health,tower* base){
	config.players[id].id=getGlobalId();
	config.players[id].isfriend=isfriend;
	config.players[id].base_health=base_health;
	config.players[id].base=base;
	if (id!=0)
		config.players_num++;
}
