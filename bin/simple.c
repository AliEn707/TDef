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
#include "../src/system_info.h"
#include "../src/statistic.h"
//Test main file

#define semInfo() printDebug("sem %d=>%d|%d=>%d|%d=>%d before sem %d action %d  %s:%d\n",0,semctl(t_sem.send,0,GETVAL),1,semctl(t_sem.send,1,GETVAL),2,semctl(t_sem.send,2,GETVAL),sem.sem_num,sem.sem_op,__FILE__,__LINE__)

#define PUBLIC_CONF "public.ini"

static int listener;

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

void segfault_sigaction(int signal, siginfo_t *si, void *arg){
	printf("Caught segfault at address %p\n", si->si_addr);
	config.game.run = 0;
	statisticsPrint();
	realizeServer();
	close(listener);
	networkPortFree();
	statisticsClear();
	usleep(100000);
	//TODO: add write info about segfault
	exit(0);
}

static void getPublicData(){
	FILE* f=fopen(PUBLIC_CONF,"rt");
	if (f==0){
		printf("Cant open %s used defaults\n",PUBLIC_CONF);
		return;
	}
	fscanf(f,"%s %d",config.game.public.host,&config.game.public.port);
	fclose(f);
}

int main(int argc, char* argv[]){
//	FILE * file;
	struct sembuf sem;
	struct sembuf sem_pl;
	
	
	int err;
	struct timeval tv={0,0};
	timePassed(&tv);
	
	struct sigaction sa;

	memset(&sa, 0, sizeof(sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = segfault_sigaction;
	sa.sa_flags   = SA_SIGINFO;

	sigaction(SIGSEGV, &sa, NULL);	
	sigaction(SIGINT, &sa, NULL);	
	
	srand(time(0));
	memset(&config,0,sizeof(config));
//	gnode grid[100];
	
	////////////////////
//	atexit(clearAll);
//	sysInit();
	//glutMainLoop();	
	memset(&sem,0,sizeof(sem));
	memset(&sem_pl,0,sizeof(sem_pl));
	
	sprintf(config.game.map,"pvz11_11");//"test");//"4");
	config.game.port=34140;

	sprintf(config.game.public.host,"localhost");//"test");//"4");
	config.game.public.port=7000;

	if (argc>1){
		parseArgv(argc,argv);
		if (config.game.token!=0) {//get game.port, game.token
			getPublicData();
			if (networkPortTake()!=0){
				printf("can't take port");
			}
			printDebug("port %d token %d\n",config.game.port,config.game.token);
		
			if (publicGetGame()<0){
				goto end;
	//			return 0;
			}
		}
		
	}
	
	printDebug("initialising\nmap %s\non port %d\n",config.game.map,config.game.port);
	gnode* grid;
	
	statisticsInit();
	initGridMath();
	//	loadConfig("../test.cfg");
//	loadTypes("../types.cfg");
	loadNpcTypes();
	loadTowerTypes();
	loadBulletTypes();
	grid=loadMap(config.game.map);
	
	allocArrays();
	config.game.wait_start=START_WAITING_TIME;
	
	listener=startServer(config.game.port,grid);
	
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
 	while(config.players_num==0)
 		usleep(100000);
	
	for (; config.game.wait_start > 0; config.game.wait_start-=START_WAITING_STEP) {
		if (config.players_num == config.game.players - 1)
			break;		
		usleep(START_WAITING_STEP);
//		printDebug("wait for players\n");
	}
	
	printDebug("start game\n");
	config.max_money_timer = TPS*60;
	pthread_t keyboardThread; //evil
	if (pthread_create(&keyboardThread, 0, printInfo, 0)!=0)
		perror("cant start keyboard thread");
	
	while(1){
		//drawGrid(grid);
		
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
		sem_pl.sem_num=0;
		sem_pl.sem_op=-1;
		t_semop(t_sem.player,&sem_pl,1);
		
		sem.sem_num=1;
		sem.sem_op=config.players_num+1;
//		semInfo();
		t_semop(t_sem.send,&sem,1);
		
		//set 0
		sem.sem_num=0;
		sem.sem_op=config.players_num;
//		semInfo();
		usleep(10);
		t_semop(t_sem.send,&sem,1);
		
		sem.sem_num=2;
		sem.sem_op=config.players_num*2;
//		semInfo();
		t_semop(t_sem.send,&sem,1);
		
		sem_pl.sem_num=0;
		sem_pl.sem_op=1;
		usleep(10);
		t_semop(t_sem.player,&sem_pl,1);
		
		syncTPS(timePassed(&tv),TPS);
		timePassed(&tv);
		if(config.players_num==0)
			break;
		//check 2
		sem.sem_num=2;
		sem.sem_op=0;
//		semInfo();
		usleep(10);
		t_semop(t_sem.send,&sem,1);
		//drop 1
		sem.sem_num=1;
		sem.sem_op=-1;
//		semInfo();
		usleep(10);
		t_semop(t_sem.send,&sem,1);
		//check 1
		sem.sem_num=1;
		sem.sem_op=0;
//		semInfo();
		err=t_semop(t_sem.send,&sem,1); 
		if (err<0)
			printDebug("t_semop err\n");
		//check 0
		sem.sem_num=0;
		sem.sem_op=0;
//		semInfo();
		err=t_semop(t_sem.send,&sem,1);
		if (err<0)
			printDebug("t_semop err\n");
		
//		if (err<0)
//			printDebug("t_semop err\n");
		
//		int z;
		//z=timePassed(1);
		//printDebug("time %d",z);
		
		
		//pinfo();
		
		//usleep(100000);
	
		config.current_money_timer++;
		playersClearBitMasks();
		forEachNpc(grid,tickMiscNpc);
		forEachTower(grid,tickMiscTower);
		forEachBullet(grid,tickMiscBullet);
	}
	statisticsPrint();
	printDebug("closing\n");
	config.game.run=0;
	close(listener);	
	
	
	realizeMap(grid);
	realizeTypes();
	realizeArrays();
	realizeServer();
	
	//add wait treads to send results
	
	
	if (config.game.token!=0){
		//send results
		publicSendResults();
	}
end:
	statisticsClear();
	//send to clear port
	networkPortFree();
	
//	memset(&config.wave_current,0,sizeof(config.wave_current));
	
//	clearAll(grid);
	
	
	return 0;
}	
