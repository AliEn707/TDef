#include "../grid.h"
#include "../gridmath.h"
#include "../engine.h"
#include "../engine_npc.h"
#include "../engine_tower.h"
#include "../engine_bullet.h"
#include "../file.h"
#include "../network.h"
#include "../threads.h"
#include "../public.h"
//Test main file

#define semInfo() printf("sem %d=>%d|%d=>%d|%d=>%d before sem %d action %d  %s:%d\n",0,semctl(config.sem.send,0,GETVAL),1,semctl(config.sem.send,1,GETVAL),2,semctl(config.sem.send,2,GETVAL),sem.sem_num,sem.sem_op,__FILE__,__LINE__)

void pinfo(){
	int i=0,
		j=0,
		k=0;
	printf("Towers\t\t\tNpcs\t\t\tBullets\n");
	while(i<config.tower_max||
		j<config.npc_max||
		k<config.bullet_max){
		for(;config.tower_array[i].id<=0 && i<config.tower_max;i++);
		if (i<config.tower_max){
			printf("%d(%d)%d ",config.tower_array[i].id,
					config.tower_array[i].position,
					config.tower_array[i].type!=BASE?
						config.tower_array[i].health:
						config.players[config.tower_array[i].owner].base_type.health
					);
			i++;
		}
		printf("|\t\t\t");
		for(;config.npc_array[j].id<=0 && j<config.npc_max;j++);
		if (j<config.npc_max){
			printf("%d(%g,%g)%d %d",config.npc_array[j].id,
					config.npc_array[j].position.x,
					config.npc_array[j].position.y,
					config.npc_array[j].health,
					config.npc_array[j].status
					);
			j++;
		}
		printf("|\t\t\t");
		for(;config.bullet_array[k].id<=0 && k<config.bullet_max;k++);
		if (k<config.bullet_max){
			printf("%d(%g,%g) ",config.bullet_array[k].id,
					config.bullet_array[k].position.x,
					config.bullet_array[k].position.y
					);
			k++;
		}
		printf("\n");
	}
	
		
}

void drawGrid(gnode* grid){
	int i,j;
	for(i=0;i<10;i++){
		for(j=0;j<10;j++)
//			printf("{%d}[%d]%d ",grid[to2d(i,j)].buildable,grid[to2d(i,j)].id,grid[to2d(i,j)].next);
			printf("%c ",
					grid[to2d(i,j)].tower!=0?
						grid[to2d(i,j)].tower->type==BASE?
							'B':
						'T':
						grid[to2d(i,j)].npcs[0]==0?
							grid[to2d(i,j)].walkable<1?
								'X':
							'O':
						'N');
		printf("\n");
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
	FILE * file;
	struct sembuf sem;
	struct sembuf sem_pl;
	short test=1;
	
	int f_token,f_sem=0,f_shmem;
	int f_port=0; //port in shared memmory
	char * f_mem=0;
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
	
	srand(time(0));
	memset(&config,0,sizeof(config));
//	gnode grid[100];
	
	////////////////////
//	atexit(clearAll);
//	sysInit();
	//glutMainLoop();	
	memset(&sem,0,sizeof(sem));
	memset(&sem_pl,0,sizeof(sem_pl));
	
	sprintf(config.game.map,"test");
	config.game.port=34140;

	if (argc>1){
		test=0;
		parseArgv(argc,argv);//get game.port, game.token
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
				if ((f_sem=semget(f_token,1,0))>0)
					if ((f_shmem=shmget(f_token, servnum*sizeof(char), 0777))>0)
						if ((f_mem=shmat(f_shmem,0,0))!=0){
							sem.sem_num=0; 
							sem.sem_op=-1; 
							semop(f_sem, &sem, 1);
							f_mem[f_port]=1;
							sem.sem_num=0; 
							sem.sem_op=1; 
							semop(f_sem, &sem, 1);
						}
		}
		printf("port %d token %d\n",config.game.port,config.game.token);
		
		if (publicGetGame()<0){
			f_mem[f_port]=0;
			goto end;
//			return 0;
		}
		
	}
	
	printf("initialising\nmap %s\non port %d\n",config.game.map,config.game.port);
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
	
	printf("start game\n");
	config.max_money_timer = TPS*60;
	
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
		
		//set 1
		sem_pl.sem_num=0;
		sem_pl.sem_op=-1;
		semop(config.sem.player,&sem_pl,1);
		
		sem.sem_num=1;
		sem.sem_op=config.players_num+1;
//		semInfo();
		semop(config.sem.send,&sem,1);
		
		//set 0
		sem.sem_num=0;
		sem.sem_op=config.players_num;
//		semInfo();
		usleep(10);
		semop(config.sem.send,&sem,1);
		
		sem.sem_num=2;
		sem.sem_op=config.players_num*2;
//		semInfo();
		semop(config.sem.send,&sem,1);
		
		sem_pl.sem_num=0;
		sem_pl.sem_op=1;
		usleep(10);
		semop(config.sem.player,&sem_pl,1);
		
		syncTPS(timePassed(&tv),TPS);
		if(config.players_num==0)
			break;
		//check 2
		sem.sem_num=2;
		sem.sem_op=0;
//		semInfo();
		usleep(10);
		semop(config.sem.send,&sem,1);
		//drop 1
		sem.sem_num=1;
		sem.sem_op=-1;
//		semInfo();
		usleep(10);
		semop(config.sem.send,&sem,1);
		//check 1
		sem.sem_num=1;
		sem.sem_op=0;
//		semInfo();
		err=semop(config.sem.send,&sem,1); 
		if (err<0)
			printf("semop err\n");
		//check 0
		sem.sem_num=0;
		sem.sem_op=0;
//		semInfo();
		err=semop(config.sem.send,&sem,1); 
		if (err<0)
			printf("semop err\n");
		
//		if (err<0)
//			printf("semop err\n");
		
//		int z;
		//z=timePassed(1);
		//printf("time %d",z);
		
		
		//pinfo();
		
		//usleep(100000);
		config.current_money_timer++;
		forEachPlayer();
		forEachNpc(grid,tickMiscNpc);
		forEachTower(grid,tickMiscTower);
		forEachBullet(grid,tickMiscBullet);
	}
	printStats();
	printf("closing\n");
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
	
	if (f_mem!=0){
		sem.sem_num=0; 
		sem.sem_op=-1; 
		semop(f_sem, &sem, 1);
		f_mem[f_port]=0;
		sem.sem_num=0; 
		sem.sem_op=1; 
		semop(f_sem, &sem, 1);
		shmdt(f_mem);
	}
	
//	memset(&config.wave_current,0,sizeof(config.wave_current));
	
//	clearAll(grid);
	
	
	return 0;
}	
