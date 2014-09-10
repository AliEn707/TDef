#include "grid.h"
#include "file.h"
#include "gridmath.h"
#include "threads.h"
#include "network.h"
#include "engine.h"
#include "engine_tower.h"
#include "engine_npc.h"
#include "engine_bullet.h"

#define sendData(x) if(send(sock,&x,sizeof(x),MSG_NOSIGNAL)<=0) return -1

#define getSem(x) semget(IPC_PRIVATE, x, 0755 | IPC_CREAT)

int recvData(int sock, void * buf, int size){
	int need=size;
	int get;
	get=recv(sock,buf,need,0);
	if (get<0)
		return -1;
	if (get==need)
		return get;
	printf("get not all\n");
	while(need>0){
		need-=get;
		if((get=recv(sock,buf+(size-need),need,0))<0)
			return 0;
	}
	return size;
}

int processMessage(worker_arg * data,char type){
	if (type==MSG_SPAWN_TOWER){
		int node_id=0;
		int t_id=0;
		if(recvData(data->sock,&node_id,sizeof(node_id))<0){
			perror("recv Message");
			return -1;
		}
		if (recvData(data->sock,&t_id,sizeof(t_id))<0){
			perror("recv Message");
			return -1;
		}
//		printf("spawn tower %d on %d\n",node_id,t_id);
		spawnTower(data->grid,node_id,data->id,t_id);
		return 0;
	}
	if (type==MSG_SPAWN_NPC){
		
		return 0;
	}
	return -1;
}

int startServer(int port,gnode * grid){
	int listener;
	struct sockaddr_in addr;
	struct sembuf sem;
	memset(&sem,0,sizeof(sem));
	
	if((listener = socket(AF_INET, SOCK_STREAM, 0))<0)
		perror("socket startServer");
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		perror("bind startServer");
	
	if(listen(listener, 1)<0)
		perror("listen startServer");
	
	if ((config.sem.send=getSem(3))<0)
		perror("get semsend startServer");
	sem.sem_num=0;
	sem.sem_op=1;
	semop(config.sem.send,&sem,1);
	
	if ((config.sem.player=getSem(1))<0)
		perror("get semsend startServer");
	sem.sem_num=0;
	sem.sem_op=1;
	semop(config.sem.player,&sem,1);
	
	config.game.run=1;
	if (startListener(listener,grid)<=0)
		perror("startListener");
	
	return listener;
}

int realizeServer(){
	int o=0;
	if (semctl(config.sem.send,0,IPC_RMID)<0){
		perror("semctl send realizeServer");
		o++;
	}
	if (semctl(config.sem.player,0,IPC_RMID)<0){
		perror("semctl player realizeServer");
		o++;
	}
	return o;
}

	
int tickSendNpc(gnode* grid,npc* n){
	int sock,id;
	sock=((worker_arg*)grid)->sock;
	id=((worker_arg*)grid)->id;
	if (sock==0)
		return 0;
	char type=MSG_NPC;
	sendData(type);
	sendData(n->id);
	
	int bit_mask=n->bit_mask;
	if (config.players[id].first_send!=0)
		bit_mask|=NPC_CREATE;
	sendData(bit_mask);
	if (checkMask(bit_mask,NPC_CREATE)){
		sendData(n->group);
		sendData(n->type);
	}
//	if(checkMask(bit_mask,NPC_POSITION) || checkMask(bit_mask,NPC_CREATE))
		sendData(n->position);
	if(checkMask(bit_mask,NPC_LEVEL) || checkMask(bit_mask,NPC_CREATE))
		sendData(n->level);
	if(checkMask(bit_mask,NPC_HEALTH) || checkMask(bit_mask,NPC_CREATE))
		sendData(n->health);
	return 0;
}


int tickSendTower(gnode* grid,tower* t){
	int sock,id;
	sock=((worker_arg*)grid)->sock;
	id=((worker_arg*)grid)->id;
	if (sock==0)
		return 0;
	char type=MSG_TOWER;
	sendData(type);
	sendData(t->id);
	
	int bit_mask=t->bit_mask;
	if (config.players[id].first_send!=0)
		bit_mask|=TOWER_CREATE;
	sendData(bit_mask);
	if(checkMask(bit_mask,TOWER_CREATE)){
		sendData(t->type);
		sendData(t->owner);
		sendData(t->position);
	}
	if(checkMask(bit_mask,TOWER_TARGET) || checkMask(bit_mask,TOWER_CREATE)){
		short target=-1;
		if(t->target!=0)
			target=getGridId(t->target->position);
		sendData(target);
	}
	if(checkMask(bit_mask,TOWER_LEVEL) || checkMask(bit_mask,TOWER_CREATE))
		sendData(t->level);
	if(checkMask(bit_mask,TOWER_HEALTH) || checkMask(bit_mask,TOWER_CREATE))
		sendData(t->health);
	return 0;
}


int tickSendBullet(gnode* grid,bullet * b){
	int sock,id;
	sock=((worker_arg*)grid)->sock;
	id=((worker_arg*)grid)->id;
	if (sock==0)
		return 0;
	char type=MSG_BULLET;
	sendData(type);
	sendData(b->id);
	
	int bit_mask=b->bit_mask;
	if (config.players[id].first_send!=0)
		bit_mask|=BULLET_CREATE;
	sendData(bit_mask);
//	if(checkMask(bit_mask,BULLET_POSITION) || checkMask(bit_mask,BULLET_CREATE))
	sendData(b->position);
	if (checkMask(bit_mask,BULLET_CREATE)){
		sendData(b->type);
		sendData(b->owner);
		sendData(b->source);
//		sendData(b->destination);
	}
	if(checkMask(bit_mask,BULLET_DETONATE))
		sendData(b->detonate);
	return 0;
}

int sendPlayers(int sock,int player){
	int i;
	if (sock==0)
		return 0;
	for(i=0;i<=config.players_num;i++)
		if(i!=player){
			if(checkMask(config.players[i].bit_mask,PLAYER_HEALTH)){
				char mes=MSG_PLAYER;
				sendData(mes);
				sendData(i);
				sendData(config.players[i].base_health);
			}
		}
	return 0;
}

int sendTest(int sock){
	char mes=MSG_TEST;
	sendData(mes);
	return 0;
}
