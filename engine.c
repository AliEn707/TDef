#include "grid.h"
#include "engine.h"
#include "engine_npc.h"
#include "engine_tower.h"
#include "engine_bullet.h"


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
	if ((config.players=malloc(sizeof(bullet)*config.player_max))==0)
		perror("malloc player initArrays");
	memset(config.players,0,sizeof(bullet)*config.player_max);
	initAreaArray();
//	printf("%d %d %d\n",sizeof(tower)*config.tower_max,sizeof(npc)*config.npc_max,sizeof(bullet)*config.bullet_max);
}

void realizeArrays(){
	free(config.players);
	free(config.tower_array);
	free(config.npc_array);
	free(config.bullet_array);
	realizeAreaArray();
}


void setupPlayer(int id,int isfriend,int base_health){
	config.players[id].id=getGlobalId();
	config.players[id].isfriend=isfriend;
	config.players[id].base_health=base_health;
}
