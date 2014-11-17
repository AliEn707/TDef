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


#define sendData(x) if(send(sock,&x,sizeof(x),0)<0) return -1

#define getSem(x) semget(IPC_PRIVATE, x, 0666 | IPC_CREAT)

// printf("worker %d sem %d=>%d|%d=>%d|%d=>%d before sem %d action %d\n",data->id,0,semctl(config.sem.send,0,GETVAL),1,semctl(config.sem.send,1,GETVAL),2,semctl(config.sem.send,2,GETVAL),sem[x].sem_num,sem[x].sem_op); 
#define semOp(x)				semop(config.sem.send,&sem[x],1)

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
	//printf("sock %d\n",data->sock);
	while(config.game.run!=0){
	//	printf("work\n");
		semOp(3);
	//	printf("sock %d\n",data->sock);
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
			processMessage(data,msg_type);
		}
		
		//all threads in one time
		semOp(3);
		sleep(0);
		semOp(4);  //thread 3 stops here
		
		if (forEachNpc((gnode*)data,tickSendNpc)<0)
			break;
		if(forEachTower((gnode*)data,tickSendTower)<0)
			break;
		if(forEachBullet((gnode*)data,tickSendBullet)<0)
			break;
		if (sendPlayers(data->sock,data->id)<0)
			break;
		//while(semctl(config.sem.send,1,GETVAL)!=config.players_num)
		semOp(1);
		sleep(0);
		semOp(2); //thread 4 stops here
		semOp(0);
		if (config.players[data->id].first_send!=0)
			config.players[data->id].first_send=0;
	}
out:
	semOp(1); //drop sem send[1]
	semOp(0);
	semOp(2);
	semop(config.sem.player,&sem_pl[0],1);
	config.players_num--;
	semop(config.sem.player,&sem_pl[1],1);
	printf("close worker\n");
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
//	printf("sock %d\n",data->sock);
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
	config.game.players=3;
	//printf("sock %d\n",data->sock);
	while(config.game.run!=0){
		FD_ZERO(&read_fds);
		FD_SET(listener, &read_fds);
		printf("wait for client\n");
		if (select (listener + 1, &read_fds, 0, 0, 0) > 0) {
			if((sock = accept(listener, NULL, NULL))<0)  //thread 2 stops here
				perror("accept startServer");
			
			if (config.players_num>=config.game.players-1){
				close(sock);
				continue;
			}
			//check connected user
			printf("client connected\n");
			semop(config.sem.player,&sem[0],1);
			config.players_num++;
			//add get player data
			
			//setup player change to get from server
			int id=config.players_num;
			printf("client id set to %d\n",id);
			/////
			
			setupPlayer(id,id/*group*/,2000/*base health*/);
	//		printf("player id %d base %d on %d \n",id,config.players[id].base_id,config.bases[config.players[id].base_id].position);
			semop(config.sem.player,&sem[1],1);
			
			
			semop(config.sem.send,&sem[2],1);
			semop(config.sem.send,&sem[3],1);
			
			setPlayerBase(id,spawnTower(data->grid,config.bases[config.players[id].base_id].position,id,BASE));
			//start worker
			if (startWorker(sock,id,data->grid)<=0)
				perror("startWorker");
			//need to change later
			//break;
		}
		syncTPS(timePassed(&tv),TPS);
	}
//	close(listener);
	free(data);
	printf("close listener\n");
	return 0;
}

pthread_t startListener(int sock, gnode* grid){
	worker_arg *data;
	printf("start listener\n");
	if ((data=malloc(sizeof(worker_arg)))==0)
		perror("malloc startListener");
	pthread_t th=0;
	data->sock=sock;
	data->grid=grid;
//	printf("sock %d\n",data->sock);
	if(pthread_create(&th,0,threadListener,data)!=0)
		return 0;
	return th;
}