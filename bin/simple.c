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

void segfault_sigaction(int signal, siginfo_t *si, void *arg)
{
	printf("Caught segfault at address %p\n", si->si_addr);
	config.game.run = 0;
	printStats();
	realizeServer();
	usleep(100000);
	exit(0);
}

int main(int argc, char* argv[]){
//	FILE * file;
	struct sembuf sem;
	struct sembuf sem_pl;
	short test=1;
	
	int listener;
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
	
	sprintf(config.game.map,"4");//"test");
	config.game.port=34140;

	if (argc>1){
		if (parseArgv(argc,argv)) {//get game.port, game.token
			test=0;
			int manager=0;
			char $_$=0;
			manager=connectToHost("localhost",7920);
			if (manager==0)
				return 0;
			printDebug("connected to manager\n");
			if (_sendData(manager,&config.game.port,sizeof(config.game.port))<=0)
				return -1;
			printDebug("sent port\n");
			if (recvData(manager,&$_$,sizeof($_$))<=0)
				return -1;
			printDebug("get %d\n",$_$);
			if ($_$!=-1)
				return -1;
			$_$=1;
			if (_sendData(manager,&$_$,sizeof($_$))<=0)
				return -1;
			printDebug("send ");
			close(manager);
	/* //TODO: repair
			if ((file=fopen("manager.ini","r"))!=0){
				char buffer[101];
				int startport; //tmp need to set f_port
				int servnum; //tmp need to size of shared memory
				while (!feof (file)) {
					if (fgets (buffer,100,file) == NULL ) 
						break;
	//				sscanf(buffer, "menport %d", &f_port);
					sscanf(buffer, "servnum  %d", &servnum);
					sscanf(buffer, "startport  %d", &startport);
				}
				fclose (file);
				f_port=config.game.port-startport;
				if ((f_token=ftok("manager.ini",100))>0)
				//	if ((f_sem=t_semget(f_token,1,0))>0) //to change
						if ((f_shmem=shmget(f_token, servnum*sizeof(char), 0777))>0)
							if ((f_mem=shmat(f_shmem,0,0))!=0){
	//							sem.sem_num=0; 
	//							sem.sem_op=-1; 
	//							t_semop(f_sem, &sem, 1);
								//TODO change to sockets
								f_mem[f_port]=1;
	//							sem.sem_num=0; 
	//							sem.sem_op=1; 
	//							t_semop(f_sem, &sem, 1);
							}
			}
	*/
			printDebug("port %d token %d\n",config.game.port,config.game.token);
		
			if (publicGetGame()<0){
				goto end;
	//			return 0;
			}
		}
		
	}
	
	printDebug("initialising\nmap %s\non port %d\n",config.game.map,config.game.port);
	gnode* grid;
	
	initGridMath();
	//	loadConfig("../test.cfg");
//	loadTypes("../types.cfg");
	loadNpcTypes();
	loadTowerTypes();
	loadBulletTypes();
	grid=loadMap(config.game.map);
	
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
	while(config.players_num==0)
		usleep(100000);
	
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
	printStats();
	printDebug("closing\n");
	config.game.run=0;
	close(listener);	
	
	
	realizeMap(grid);
	realizeTypes();
	realizeArrays();
	realizeServer();
	
	//add wait treads to send results
	
	
	if (test==0){
		//send results
		publicSendResults();
	}
end:
	//send to clear port
	if (test==0){
		int manager=0;
		char $_$=0;
		manager=connectToHost("localhost",7920);
		if (manager!=0){
			if (_sendData(manager,&config.game.port,sizeof(config.game.port))<=0)
				return -1;
			if (_sendData(manager,&$_$,sizeof($_$))<=0)
				return -1;
			close(manager);
		}
	}
	
//	memset(&config.wave_current,0,sizeof(config.wave_current));
	
//	clearAll(grid);
	
	
	return 0;
}	
