#include <sys/socket.h>
#include <netinet/in.h>

#include "grid.h"
#include "threads.h"
#include "file.h"
#include "gridmath.h"
#include "network.h"
#include "engine.h"
#include "engine_npc.h"
#include "engine_tower.h"
#include "engine_bullet.h"
#include "types.h"
#include "t_sem.h"

#define PRIVATE_POLICY "<cross-domain-policy><allow-access-from domain=\"*\" to-ports=\"*\" /></cross-domain-policy>"

#define sendData(x) if(send(sock,&x,sizeof(x),0)<0) return -1

#define getSem(x) semget(IPC_PRIVATE, x, 0666 | IPC_CREAT)

// printDebug("worker %d sem %d=>%d|%d=>%d|%d=>%d before sem %d action %d\n",data->id,0,semctl(t_sem.send,0,GETVAL),1,semctl(t_sem.send,1,GETVAL),2,semctl(t_sem.send,2,GETVAL),sem[x].sem_num,sem[x].sem_op); 
#define semOp(x)				t_semop(t_sem.send,&sem[x],1)

/// worker thread get data from server and change world
void * threadWorker(void * arg){
	worker_arg *data=arg;
	struct sembuf sem[5]={{0,-1,0},
						{1,-1,0},
						{1,0,0},
						{2,-1,0},
						{2,0,0}};
	struct sembuf sem_pl[2]={{0,-1,0},
						   {0,1,0}};
	int i;
	char msg_type;
	config.players[data->id].first_send=1;
	//send start data
	if (networkAuth(data)!=0)
		return (void *)-1;
	//printDebug("sock %d\n",data->sock);
	//TODO: add send data about player
	sendPlayers(data->sock,data->id);
	while(config.game.wait_start>0){
		networkWaitingTime(data);
		usleep(START_WAITING_STEP);
	}
	int error=0;
	while(config.game.run!=0){
	//	printDebug("work\n");
		usleep(10000);
		semOp(3);
	//	printDebug("sock %d\n",data->sock);
		for(i=0;i<10;i++){
			if (recv(data->sock,&msg_type,sizeof(msg_type),MSG_DONTWAIT)<0){
				if (errno==EAGAIN){
					sleep(0);
					continue;
				}else{
					perror("recv threadWorker");
					semOp(3);
					goto out;
					break;
				}
			}
			//TODO: add check for errors and drop
			if(processMessage(data,msg_type)<0){
				error++;
				break;
			}
		}
		//set creation mark on new objects
		if (config.players[data->id].first_send!=0){ //maybe not need
			setMask(&config.players[data->id], PLAYER_CREATE);
			setMask(config.players[data->id].base, TOWER_CREATE);
			setMask(config.players[data->id].hero, NPC_CREATE);
		}
		//all threads in one time
		semOp(3);
		sleep(0);
		semOp(4);  //thread 3 stops here
		if (error)
			break;
		if (forEachNpc((gnode*)data,tickSendNpc)<0)
			break;
		if(forEachTower((gnode*)data,tickSendTower)<0)
			break;
		if(forEachBullet((gnode*)data,tickSendBullet)<0)
			break;
		if (sendPlayers(data->sock,data->id)<0)
			break;
		//while(semctl(t_sem.send,1,GETVAL)!=config.players_num)
		semOp(1);
		sleep(0);
		semOp(2); //thread 4 stops here
		semOp(0);
		//need to check
		if (config.players[data->id].first_send!=0)
			config.players[data->id].first_send=0;
	}
out:
	semOp(1); //drop sem send[1]
	semOp(0);
//	semOp(2);  //need to test, may be need
	//go for one another iteration
	semOp(3);
	//here we can do different things, after that will be sending to clients
	if (config.players[data->id].base!=0){
		removeTower(data->grid,config.players[data->id].base);
	}
	if (config.players[data->id].hero!=0){
		config.players[data->id].hero->health = -1;
//		config.players[data->id].hero->last_attack=0;
		setMask(config.players[data->id].hero,NPC_HEALTH);
	}
	
	usleep(10000);
	semOp(3);
	semOp(4);  
	usleep(10000);
	semOp(1);
	sleep(0);
	semOp(2); //thread 4 stops here
	semOp(0);
	
	t_semop(t_sem.player,&sem_pl[0],1);
	config.players_num--;
	t_semop(t_sem.player,&sem_pl[1],1);
	
	//add send stats to public
	
	config.players[data->id].id=0;	
	if (config.players[data->id].base!=0){
		data->grid[config.players[data->id].base->position].tower=0;
		config.players[data->id].base=0;
	}
	
//	memset(&config.players[data->id],0,sizeof(player));
	printDebug("close worker\n");
	free(data);
	return 0;
}

pthread_t startWorker(int sock,int id,gnode *grid){
	worker_arg *data;
	if ((data=malloc(sizeof(worker_arg)))==0)
		perror("malloc startWorker");
	pthread_t th=0;
	data->sock=sock;
	data->id=id;
	data->grid=grid;
//	printDebug("sock %d\n",data->sock);
	if(pthread_create(&th,0,threadWorker,data)!=0)
		return 0;
	return th;
}

//listen to new players
void * threadListener(void * arg){
	worker_arg *data=arg;
	int listener=data->sock;
	int sock;
	fd_set read_fds;
	struct sembuf sem[4]={{0,-1,0},
						{0,1,0},
						{2,-1,0},
						{2,1,0}};
	struct timeval tv={0,0};
	timePassed(&tv);
	config.players_num=0;
	//
	//config.game.players=5;
	//printDebug("sock %d\n",data->sock);
	while(config.game.run!=0){
		FD_ZERO(&read_fds);
		FD_SET(listener, &read_fds);
		printDebug("wait for client\n");
		if (select (listener + 1, &read_fds, 0, 0, 0) > 0) {
			if((sock = accept(listener, NULL, NULL))<0)  //thread 2 stops here
				perror("accept startServer");
			char t_t[14];
			memset(t_t,0,sizeof(t_t));
			recvData(sock,t_t,13);//get 13 bytes
//			printf("%s\n",t_t);
			if (strstr(t_t,"<policy")!=0){
				_sendData(sock,PRIVATE_POLICY,sizeof(PRIVATE_POLICY));
				close(sock);
			}else{
				if (config.players_num>=config.game.players-1){
					close(sock);
					continue;
				}
				//check connected user
				printDebug("client connected\n");
				t_semop(t_sem.player,&sem[0],1);
				config.players_num++;
				//add get player data
				
	//////////////////////////setup player change to get from server
					int i;
					int id;
					id=config.players_num;
					for (i=1;i<=config.game.players;i++)
						if (config.players[i].id==0){
							id=i;
							break;
						}
					printDebug("client id set to %d\n",id);
					/////important, not remove
					setupPlayer(id,id/*group*/);
					
					//fake setup base
					config.players[id].base_type.health=2000;//base health
					//fake setup hero
					npc_type * type=typesNpcGet(HERO);
					if (type!=0)
						memcpy(&config.players[id].hero_type,type,sizeof(npc_type));
					printDebug("debug worker\n",id);
					config.players[id].money = 1000;//TODO:remove!
	///////////////////////////////////////////
	//			printDebug("player id %d base %d on %d \n",id,config.players[id].base_id,config.bases[config.players[id].base_id].position);
				t_semop(t_sem.player,&sem[1],1);
				printDebug("debug worker\n",id);
				
//				t_semop(t_sem.send,&sem[2],1);
//				t_semop(t_sem.send,&sem[3],1);
				tower * base=spawnTower(data->grid,config.bases[config.players[id].base_id].position,id,BASE);
				setPlayerBase(id,base);
				npc * hero=spawnNpc(data->grid,config.points[config.bases[config.players[id].base_id].point_id].position,id,HERO);
				setPlayerHero(id,hero);
				printDebug("starting worker\n",id);
				//start worker
				if (startWorker(sock,id,data->grid)<=0)
					perror("startWorker");
				//need to change later
				//break;
			}
		}
		syncTPS(timePassed(&tv),TPS);
	}
//	close(listener);
	printDebug("close listener\n");
	free(data);
	return 0;
}

pthread_t startListener(int sock, gnode* grid){
	worker_arg *data;
	printDebug("start listener\n");
	if ((data=malloc(sizeof(worker_arg)))==0)
		perror("malloc startListener");
	pthread_t th=0;
	data->sock=sock;
	data->grid=grid;
//	printDebug("sock %d\n",data->sock);
	if(pthread_create(&th,0,threadListener,data)!=0)
		return 0;
	return th;
}
