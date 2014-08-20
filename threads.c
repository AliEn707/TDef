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
	while(config.game!=0){
		semOp(0);
	//	printf("sock %d\n",data->sock);
		if (forEachNpc((gnode*)&data->sock,tickSendNpc)<0)
			break;
		if(forEachTower((gnode*)&data->sock,tickSendTower)<0)
			break;
		if(forEachBullet((gnode*)&data->sock,tickSendBullet)<0)
			break;
		semOp(1);
		semOp(2);
	}
	config.players_num--;
	
	free(data);
	return 0;
}

pthread_t startWorker(int sock){
	worker_arg *data=malloc(sizeof(worker_arg));
	pthread_t th=0;
	data->sock=sock;
	printf("sock %d\n",data->sock);
	if(pthread_create(&th,0,threadWorker,data)!=0)
		return 0;
	return th;
}