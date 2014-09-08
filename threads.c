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

#define semOp(x) semop(config.sem.send,&sem[x],1)

/// worker thread get data from server and change world
void * threadWorker(void * arg){
	worker_arg *data=arg;
	struct sembuf sem[3]={{0,0,0},
					 {1,-1,0},
					 {2,0,0}};
	struct sembuf sem_pl[2]={{0,-1,0},
					     {0,1,0}};
	int i;
	char msg_type;
	config.players[data->id].first_send=1;
	//printf("sock %d\n",data->sock);
	while(config.game.run!=0){
	//	printf("work\n");
		semOp(0);
	//	printf("sock %d\n",data->sock);
		for(i=0;i<10;i++){
			if (recv(data->sock,&msg_type,sizeof(msg_type),MSG_DONTWAIT)<0){
				if (errno==EAGAIN){
					sleep(0);
					continue;
				}else{
					perror("recv threadWorker");
					break;
				}
			}
			processMessage(data,msg_type);
		}
		if (forEachNpc((gnode*)data,tickSendNpc)<0)
			break;
		if(forEachTower((gnode*)data,tickSendTower)<0)
			break;
		if(forEachBullet((gnode*)data,tickSendBullet)<0)
			break;
		//while(semctl(config.sem.send,1,GETVAL)!=config.players_num)
		semOp(1);
		semOp(2);
		if (config.players[data->id].first_send!=0)
			config.players[data->id].first_send=0;
	}
	semOp(1); //drop sem send[1]
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
	struct sembuf sem[2]={{0,-1,0},
					{0,1,0}};
	config.players_num=0;
	//
	config.game.players=2;
	//printf("sock %d\n",data->sock);
	while(config.game.run!=0){
		printf("wait for client\n");
		
		if((sock = accept(listener, NULL, NULL))<0)
			perror("accept startServer");
		if (config.players_num>=config.game.players){
			close(sock);
			continue;
		}
		//check connected user
		printf("client connected\n");
		//add get player data
		
		//setup player
		semop(config.sem.player,&sem[0],1);
		setupPlayer(1,1,2000,0);
		semop(config.sem.player,&sem[1],1);
		//start worker
		if (startWorker(sock,1,data->grid)<=0)
			perror("startWorker");
		//need to change later
		//break;
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