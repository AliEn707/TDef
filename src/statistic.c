#include "grid.h"
#include "bintree.h"
#include "engine.h"

#include "statistic.h"

/*
╔══════════════════════════════════════════════════════════════╗
║ 											                       ║
║ created by Dennis Yarikov						                       ║
║ aug 2015									                       ║
╚══════════════════════════════════════════════════════════════╝
*/

#define getOrAdd(cur,from,key) \
	do{\
		cur=bintreeGet(from,key); \
		if (cur==0){ \
			if((cur=malloc(sizeof(*cur)))==0) \
				perror("malloc cache"); \
			memset(cur,0,sizeof(*cur)); \
			bintreeAdd(from,key,cur); \
		}; \
	}while(0)

static struct{
	bintree npc_spawned; //bintree of int
	bintree npc_killed; //bintree of bintree of int
	bintree npc_lost; //bintree of bintree of int
	bintree tower_built; //bintree of int
	bintree tower_destroyed; //bintree of bintree of int
	bintree tower_lost; //bintree of bintree of int
	int level;
} statistics[PLAYER_MAX];


void statisticsPrint(){
	int aa;
	for (aa = 0; aa < config.game.players; aa++) {//watch stats
		printDebug("Stats: player %d\nnpcs spawned: %d\ntowers built: %d\nnpcs killed: %d\ntowers destroyed: %d\nnpcs lost: %d\ntowers lost: %d\nxp: %d\nlevel: %d\n\n", 
		aa, config.players[aa].stat.npcs_spawned, config.players[aa].stat.towers_built,
		config.players[aa].stat.npcs_killed, config.players[aa].stat.towers_destroyed, 
		config.players[aa].stat.npcs_lost,config.players[aa].stat.towers_lost, config.players[aa].stat.xp,config.players[aa].level);
	}
}

void statisticsInit(){
	memset(statistics,0,sizeof(statistics));
}	

void statisticsClear(){
	int i;
	void clearBN(void* a){
		bintreeErase(a,free);
	}
	for(i=0;i<PLAYER_MAX;i++){
		bintreeErase(&statistics[i].npc_spawned,free);
		bintreeErase(&statistics[i].npc_killed,clearBN);
		bintreeErase(&statistics[i].npc_lost,clearBN);
		bintreeErase(&statistics[i].tower_built,free);
		bintreeErase(&statistics[i].tower_destroyed,clearBN);
		bintreeErase(&statistics[i].tower_lost,clearBN);
	}
}

void statisticAddLevel(int player){
	statistics[player].level++;
}

void statisticAddNpcSpawned(int player,int type){
	int *curr;
	getOrAdd(curr,&statistics[player].npc_spawned,type);
	(*curr)++;
}

void statisticAddNpcKilled(int player,int type, int level){
	bintree *curr;
	int *num;
	getOrAdd(curr,&statistics[player].npc_killed,type);
	getOrAdd(num,curr,level);
	(*num)++;
}

void statisticAddNpcLost(int player,int type, int level){
	bintree *curr;
	int *num;
	getOrAdd(curr,&statistics[player].npc_lost,type);
	getOrAdd(num,curr,level);
	(*num)++;
}

void statisticAddTowerBuilt(int player,int type){
	int *curr;
	getOrAdd(curr,&statistics[player].tower_built,type);
	(*curr)++;
}

void statisticAddTowerDestroed(int player,int type, int level){
	bintree *curr;
	int *num;
	getOrAdd(curr,&statistics[player].tower_destroyed,type);
	getOrAdd(num,curr,level);
	(*num)++;
}

void statisticAddTowerLost(int player,int type, int level){
	bintree *curr;
	int *num;
	getOrAdd(curr,&statistics[player].tower_lost,type);
	getOrAdd(num,curr,level);
	(*num)++;
}

