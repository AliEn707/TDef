#include <stdarg.h>
#include "grid.h"
#include "engine.h"
#include "engine_npc.h"
#include "engine_tower.h"
#include "engine_bullet.h"
#include "areaarray.h"

int parseArgv(int argc,char * argv[]){
	int i, ret = 0;
	for(i=0;i<argc;i++){
		if (strcmp(argv[i],"-port")==0){
			config.game.port=atoi(argv[++i]);
			ret |= 1;
			continue;
		}
		if (strcmp(argv[i],"-token")==0){
			config.game.token=atoi(argv[++i]);
			ret |= 2;
			continue;
		}
		if (strcmp(argv[i],"-debug")==0){
			config.debug = 1;
			continue;
		}		
	}
	return ret;
}

//time passed after previous call of function
int timePassed(struct timeval * t){
	//config.time  struct timeval
	struct timeval end;
	gettimeofday(&end, NULL);
	int out=((end.tv_sec - t->tv_sec)*1000000+
			end.tv_usec - t->tv_usec);
	memcpy(t,&end,sizeof(end));
	return out;
}

void syncTPS(int z,int _TPS){
	if((z=(1000000/_TPS)-z)>0){
		usleep(z);
	}
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
//	printDebug("%d %d %d\n",sizeof(tower)*config.tower_max,sizeof(npc)*config.npc_max,sizeof(bullet)*config.bullet_max);
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
	int current;
//	printDebug("%g %g %g %g\n",x1,y1,x2,y2);
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
		 
		for(;x1<x2;x1+=0.5){
			y1=K*x1+B;
//			printDebug("1} %d\n",to2d(((int)x1),((int)y1)));
			if ((current=to2d(((int)x1),((int)y1)))!=destination){
				if (grid[current].walkable<0){
//					printDebug("!\n");
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
		 
		for(;y1<y2;y1+=0.5){
			x1=K*y1+B;
//			printDebug("2} %d\n",to2d(((int)x1),((int)y1)));
			if ((current=to2d(((int)x1),((int)y1)))!=destination){
				if (grid[current].walkable<0)
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
			typeof(x1) tmp;
			tmp=x1;
			x1=x2;
			x2=tmp;
			tmp=y1;
			y1=y2;
			y2=tmp;
		}
		float K=(y2-y1)/(x2-x1);
		float B=(y1*(x2-x1)-x1*(y2-y1))/(x2-x1);
		
		for(;x1<=x2;x1+=0.35){
			y1=K*x1+B;
			if (to2d(((int)x1),((int)y1))!=destination){
				if (grid[to2d(((int)x1),((int)y1))].walkable<=0||
						grid[to2d(((int)x1),((int)y1))].tower>0)
					return -1;
			}
		}
	}else{
		if (y1>y2){
			typeof(x1) tmp;
			tmp=x1;
			x1=x2;
			x2=tmp;
			tmp=y1;
			y1=y2;
			y2=tmp;
		}
		float K=(x2-x1)/(y2-y1);
		float B=(x1*(y2-y1)-y1*(x2-x1))/(y2-y1);
		 
		for(;y1<=y2;y1+=0.35){
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
void setupPlayer(int id,int group){
	config.players[id].id=getGlobalId();
	config.players[id].group=group;
	config.players[id].base_id=id;
//	config.players[id].base_type.health=base_health;
}

void setPlayerBase(int id,tower* base){
	config.players[id].base=base;
	base->owner=id;
	setTowerBase(base);
}

void setPlayerHero(int id,npc* hero){
	config.players[id].hero=hero;
	hero->owner=id;
	setNpcBase(hero);
}

static int giveMoney(player *pl) {
	setMask(pl, PLAYER_MONEY);
	return 10*(pow(pl->level, 1.2) + 1);
}

static int needLevelInc(player *pl) {
	return pl->stat.xp > pow(1.2, pl->level)*1000;
}

void forEachPlayer(gnode* grid) {
	int i, money_flag = 0;
	if (config.current_money_timer%(config.max_money_timer + 1) == config.max_money_timer) {
		money_flag = 1; //need to give money!
	}	
	for (i = 0; i < config.game.players; i++) {
		if (config.players[i].id==0)
			continue;
//		config.players[i].bit_mask = 0;
		if (needLevelInc (&config.players[i])) {
			printDebug("player = %d level = %d\n", i, config.players[i].level);
			config.players[i].level++;
			setMask(&config.players[i],PLAYER_LEVEL);
		}
		if (money_flag)
			config.players[i].money += giveMoney(&config.players[i]);
		if (config.players[i].hero==0){
			if (config.players[i].hero_counter>config.players[i]._hero_counter){
				setPlayerHero(i,spawnNpc(grid,config.points[config.bases[config.players[i].base_id].point_id].position,i,HERO));
				config.players[i].hero_counter=0;
				setMask(&config.players[i],PLAYER_HERO);
			}else{
				config.players[i].hero_counter++;
				//if (config.players[i].hero_counter%TPS==0)//set less solid
				setMask(&config.players[i],PLAYER_HERO_COUNTER);
			}
		}
		config.players[i].target_changed=0;
	}
	if (money_flag)
		config.current_money_timer = 0;
}

void playersClearBitMasks(){
	int i;
	for (i = 0; i < config.game.players; i++) {
		if (config.players[i].id==0)
			continue;
		config.players[i].bit_mask = 0;
	}
} 

void printStats() {
	int aa;
	for (aa = 0; aa < config.game.players; aa++) {//watch stats
		printf("Stats: player %d\nnpcs spawned: %d\ntowers built: %d\nnpcs killed: %d\ntowers destroyed: %d\nnpcs lost: %d\ntowers lost: %d\nxp: %d\nlevel: %d\n\n", 
		aa, config.players[aa].stat.npcs_spawned, config.players[aa].stat.towers_built,
		config.players[aa].stat.npcs_killed, config.players[aa].stat.towers_destroyed, 
		config.players[aa].stat.npcs_lost,config.players[aa].stat.towers_lost, config.players[aa].stat.xp,config.players[aa].level);
	}
}

void printDebug(const char* format, ...) {
	if (config.debug == 0)
		return;
	va_list argptr;
	va_start(argptr, format);
	vfprintf(stdout, format, argptr);
	va_end(argptr);	
}

