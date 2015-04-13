#include "../src/grid.h"
#include "../src/gridmath.h"
#include "../src/engine.h"
#include "../src/engine_npc.h"
#include "../src/engine_tower.h"
#include "../src/engine_bullet.h"
#include "../src/file.h"
#include "../src/network.h"
#include "../src/threads.h"
#include "../src/public.h"
#include "../src/t_sem.h"
//Test main file

#include <signal.h>

#define semInfo() printDebug("sem %d=>%d|%d=>%d|%d=>%d before sem %d action %d  %s:%d\n",0,semctl(t_sem.send,0,GETVAL),1,semctl(t_sem.send,1,GETVAL),2,semctl(t_sem.send,2,GETVAL),sem.sem_num,sem.sem_op,__FILE__,__LINE__)


void* printInfo(void*p) {
	while (1) {
		char c = getchar();
		if (c == 'i') { //print info
			int aa;
			printf("Current info:\n\n");
			for (aa = 0; aa < config.game.players; aa++) {
				printf("Player %d\n money %d\n npcs %d\n\n", aa, config.players[aa].money, config.players[aa].stat.npcs_spawned - config.players[aa].stat.npcs_lost);
			}
		}
	}
	return 0;
}

void pinfo(){
	int i=0,
		j=0,
		k=0;
	printDebug("Towers\t\t\tNpcs\t\t\tBullets\n");
	while(i<config.tower_max||
		j<config.npc_max||
		k<config.bullet_max){
		for(;config.tower_array[i].id<=0 && i<config.tower_max;i++);
		if (i<config.tower_max){
			printDebug("%d(%d)%d ",config.tower_array[i].id,
					config.tower_array[i].position,
					config.tower_array[i].type!=BASE?
						config.tower_array[i].health:
						config.players[config.tower_array[i].owner].base_type.health
					);
			i++;
		}
		printDebug("|\t\t\t");
		for(;config.npc_array[j].id<=0 && j<config.npc_max;j++);
		if (j<config.npc_max){
			printDebug("%d(%g,%g)%d %d",config.npc_array[j].id,
					config.npc_array[j].position.x,
					config.npc_array[j].position.y,
					config.npc_array[j].health,
					config.npc_array[j].status
					);
			j++;
		}
		printDebug("|\t\t\t");
		for(;config.bullet_array[k].id<=0 && k<config.bullet_max;k++);
		if (k<config.bullet_max){
			printDebug("%d(%g,%g) ",config.bullet_array[k].id,
					config.bullet_array[k].position.x,
					config.bullet_array[k].position.y
					);
			k++;
		}
		printDebug("\n");
	}
	
		
}

void drawGrid(gnode* grid){
	int i,j;
	for(i=0;i<10;i++){
		for(j=0;j<10;j++)
//			printDebug("{%d}[%d]%d ",grid[to2d(i,j)].buildable,grid[to2d(i,j)].id,grid[to2d(i,j)].next);
			printDebug("%c ",
					grid[to2d(i,j)].tower!=0?
						grid[to2d(i,j)].tower->type==BASE?
							'B':
						'T':
						grid[to2d(i,j)].npcs[0]==0?
							grid[to2d(i,j)].walkable<1?
								'X':
							'O':
						'N');
		printDebug("\n");
	}		
}


void fillData(gnode * grid){
	int id,i;
	printf("players %d\n",config.game.players);
	for(id=0;id<config.game.players;id++){
		setupPlayer(id,id);
		config.players[id].base_type.health=2000;
		config.players[id].money = 1000;
		tower * base=spawnTower(grid,config.bases[config.players[id].base_id].position,id,BASE);
		setPlayerBase(id,base);
		npc * hero=spawnNpc(grid,config.points[config.bases[config.players[id].base_id].point_id].position,id,HERO);
		setPlayerHero(id,hero);
		for (i=0;i<30;i++)
			spawnNpc(grid,
					config.points[config.bases[config.players[id].base_id].point_id].position,
					id,
					config.players[id].npc_set[rand()%6].id);
	}
}

void check(gnode * grid){
	
}

int startRoom(){
	struct timeval tv={0,0};
	int i,j;
	gnode* grid;
	
	srand(time(0));
	memset(&config,0,sizeof(config));
	sprintf(config.game.map,"4");//"test");
	
	initGridMath();
	loadNpcTypes();
	loadTowerTypes();
	loadBulletTypes();
	grid=loadMap(config.game.map);
	
	config.game.wait_start=START_WAITING_TIME;
	for (i=0;i<5;i++){
		memset(config.players[i].tower_set,0,sizeof(config.players[i].tower_set));
		memset(config.players[i].npc_set,0,sizeof(config.players[i].npc_set));
		config.players[i]._hero_counter=TPS*62;
//		config.players[i].target=-1;
		for (j=0;j<9;j++){
			config.players[i].tower_set[j].id=j+1;
			config.players[i].tower_set[j].num=-1;
			config.players[i].npc_set[j].id=j+1;
			config.players[i].npc_set[j].num=-1;
		}
	}
	fillData(grid);
	timePassed(&tv);
	double a,b,c;
		for (a=0;a<100000;a+=0.1)
			for (b=0;b<10000;b+=0.1)
				c=pow(sqrt(a),b);
		char z[100];
		sprintf(z,"%lf",c);
		
	while(0){
		//drawGrid(grid);
		check(grid);
		processWaves(grid);
		
		forEachNpc(grid,tickDiedCheckNpc);
		forEachTower(grid,tickDiedCheckTower);
		forEachBullet(grid,tickDiedCheckBullet);

		forEachNpc(grid,tickCleanNpc);
		forEachTower(grid,tickCleanTower);
		forEachBullet(grid,tickCleanBullet);
			
		forEachNpc(grid,tickMoveNpc);
		forEachNpc(grid,tickTargetNpc);
		forEachNpc(grid,tickAttackNpc);
		
		forEachTower(grid,tickAttackTower);
		
		forEachBullet(grid,tickProcessBullet);
		
		forEachPlayer(grid);
		
		//set 1
		syncTPS(timePassed(&tv),TPS);
		timePassed(&tv);

		config.current_money_timer++;
		playersClearBitMasks();
		forEachNpc(grid,tickMiscNpc);
		forEachTower(grid,tickMiscTower);
		forEachBullet(grid,tickMiscBullet);
	}
	return 0;
}
int children[1000];
int child=0;
int bad=0;

int main(int argc, char* argv[]){
//	FILE * file;


	struct timeval tv={0,0};
	timePassed(&tv);
	
	srand(time(0));
	memset(&config,0,sizeof(config));
//	gnode grid[100];
	
	////////////////////
//	atexit(clearAll);
//	sysInit();
	//glutMainLoop();	
//	memset(&sem,0,sizeof(sem));
//	memset(&sem_pl,0,sizeof(sem_pl));
	
	sprintf(config.game.map,"4");//"test");
	config.game.port=34140;

	
	
//	printDebug("initialising\nmap %s\non port %d\n",config.game.map,config.game.port);
	gnode* grid;
	
	initGridMath();
	//	loadConfig("../test.cfg");
//	loadTypes("../types.cfg");
	loadNpcTypes();
	loadTowerTypes();
	loadBulletTypes();
	grid=loadMap(config.game.map);
	
	config.game.wait_start=START_WAITING_TIME;
	
//	listener=startServer(config.game.port,grid);
	
	//config.player_max=4;
	//	timePassed(0);
	//npc* n=
//	spawnNpc(grid,4,0,1);
	//npc* n2=
//	spawnNpc(grid,5,0,2);
//	spawnNpc(grid,6,0,3);
//	setupPlayer(2,0,1800,0);
//	spawnTower(grid,75,1,BASE);
//	spawnTower(grid,22,1,2);
	int i,j;
	//for testing change to get from server
	for (i=0;i<5;i++){
		memset(config.players[i].tower_set,0,sizeof(config.players[i].tower_set));
		memset(config.players[i].npc_set,0,sizeof(config.players[i].npc_set));
		config.players[i]._hero_counter=TPS*62;
//		config.players[i].target=-1;
		for (j=0;j<9;j++){
			config.players[i].tower_set[j].id=j+1;
			config.players[i].tower_set[j].num=-1;
			config.players[i].npc_set[j].id=j+1;
			config.players[i].npc_set[j].num=-1;
		}
	}
	//npc* n3=
//	spawnNpc(grid,42,0,2);
	
	//change in future
	printDebug("Max number of players = %d\n", config.game.players - 1); //1 for AI
 //	while(config.players_num==0)
 //		usleep(100000);
	
//	for (; config.game.wait_start > 0; config.game.wait_start-=START_WAITING_STEP) {
//		if (config.players_num == config.game.players - 1)
//			break;		
//		usleep(START_WAITING_STEP);
//		printDebug("wait for players\n");
//	}
	fillData(grid);
	printDebug("start game\n");
	config.max_money_timer = TPS*60;
//	pthread_t keyboardThread; //evil
//	if (pthread_create(&keyboardThread, 0, printInfo, 0)!=0)
//		perror("cant start keyboard thread");
	int t=time(0);
	int wait=0;
	int rooms=1;
	while(1){
		//drawGrid(grid);
		check(grid);
		
		processWaves(grid);
		
		forEachNpc(grid,tickDiedCheckNpc);
		forEachTower(grid,tickDiedCheckTower);
		forEachBullet(grid,tickDiedCheckBullet);

		forEachNpc(grid,tickCleanNpc);
		forEachTower(grid,tickCleanTower);
		forEachBullet(grid,tickCleanBullet);
			
		forEachNpc(grid,tickMoveNpc);
		forEachNpc(grid,tickTargetNpc);
		forEachNpc(grid,tickAttackNpc);
		
		forEachTower(grid,tickAttackTower);
		
		forEachBullet(grid,tickProcessBullet);
		
		forEachPlayer(grid);
		
		//set 1
		wait=syncTPS(timePassed(&tv),TPS);
		timePassed(&tv);

		config.current_money_timer++;
		playersClearBitMasks();
		forEachNpc(grid,tickMiscNpc);
		forEachTower(grid,tickMiscTower);
		forEachBullet(grid,tickMiscBullet);
		
		if (time(0)-t>1){
			printf("time out: wait now %d\n",wait);
			t=time(0);
			int z;
			if((z=fork())==0){
				startRoom();
			}else{
				rooms++;
				children[child++]=z;
			}
			
		}
		if (wait<-100000) //200ms not very bad
			bad++;
		else
			bad=0;
		
		if (bad>6)
			break;
	}
	printf("closing on %d\n",rooms);
	config.game.run=0;
//	close(listener);	
	
	
	realizeMap(grid);
	realizeTypes();
	realizeArrays();
	
	//add wait treads to send results
	for(i=0;i<child;i++){
		kill(children[i],SIGINT);
	}

//	memset(&config.wave_current,0,sizeof(config.wave_current));
	
//	clearAll(grid);
	
	
	return 0;
}	
