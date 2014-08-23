#include <sys/socket.h>
#include <netinet/in.h>

#include "threads.h"
#include "grid.h"
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

void * threadWorker(void * arg){
	worker_arg *data=arg;
	struct sembuf sem[3]={{0,0,0},
					{1,-1,0},
					{2,0,0}};
	
	//printf("sock %d\n",data->sock);
	while(config.game.run!=0){
	//	printf("work\n");
		semOp(0);
	//	printf("sock %d\n",data->sock);
		if (forEachNpc((gnode*)&data->sock,tickSendNpc)<0)
			break;
		if(forEachTower((gnode*)&data->sock,tickSendTower)<0)
			break;
		if(forEachBullet((gnode*)&data->sock,tickSendBullet)<0)
			break;
		//while(semctl(config.sem.send,1,GETVAL)!=config.players_num)
		semOp(1);
		semOp(2);
	}
	config.players_num--;
	printf("close worker\n");
	free(data);
	return 0;
}

pthread_t startWorker(int sock){
	worker_arg *data;
	if ((data=malloc(sizeof(worker_arg)))==0)
		perror("malloc startWorker");
	pthread_t th=0;
	data->sock=sock;
	printf("sock %d\n",data->sock);
	if(pthread_create(&th,0,threadWorker,data)!=0)
		return 0;
	return th;
}

void * threadListener(void * arg){
	worker_arg *data=arg;
	int listener=data->sock;
	int sock;
	struct sembuf sem[2]={{0,-1,0},
					{0,1,0}};
	config.players_num=0;
	//
	config.game.players=1;
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
		//start worker
		semop(config.sem.player,&sem[0],1);
		setupPlayer(1,1,2000,0);
		semop(config.sem.player,&sem[1],1);
		if (startWorker(sock)<=0)
			perror("startWorker");
		//need to change later
		//break;
	}
//	close(listener);
	free(data);
	printf("close listener\n");
	return 0;
}

pthread_t startListener(int sock){
	worker_arg *data;
	printf("start listener\n");
	if ((data=malloc(sizeof(worker_arg)))==0)
		perror("malloc startListener");
	pthread_t th=0;
	data->sock=sock;
//	printf("sock %d\n",data->sock);
	if(pthread_create(&th,0,threadListener,data)!=0)
		return 0;
	return th;
}