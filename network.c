#include <sys/socket.h>
#include <netinet/in.h>

#include "grid.h"
#include "file.h"
#include "gridmath.h"

#define sendData(x) if(send(sock,&x,sizeof(x),0)<0) return -1

#define getSem(x) semget(IPC_PRIVATE, x, 0755 | IPC_CREAT)



int startServer(int port){
	int sock, listener;
	struct sockaddr_in addr;
	
	if((listener = socket(AF_INET, SOCK_STREAM, 0))<0)
		perror("socket startServer");
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		perror("bind startServer");
	
	if(listen(listener, 1)<0)
		perror("listen startServer");
	
	config.sem.send=getSem(3);
	
	return listener;
}

int realizeServer(){
	semctl(config.sem.send,0,IPC_RMID);
}

	
int tickSendNpc(gnode* grid,npc* n){
	int sock;
	sock=*((int*)grid);
	if (sock==0)
		return;
	char type=MSG_NPC;
	sendData(type);
	sendData(n->id);
	sendData(n->bit_mask);
	if (checkMask(n,NPC_CREATE)){
		sendData(n->isfriend);
		sendData(n->type);
	}
//	if(checkMask(n,NPC_POSITION) || checkMask(n,NPC_CREATE))
		sendData(n->position);
	if(checkMask(n,NPC_HEALTH) || checkMask(n,NPC_CREATE))
		sendData(n->health);
	return 0;
}


int tickSendTower(gnode* grid,tower* t){
	int sock;
	sock=*((int*)grid);
	if (sock==0)
		return;
	char type=MSG_TOWER;
	sendData(type);
	sendData(t->id);
	sendData(t->bit_mask);
	if(checkMask(t,TOWER_CREATE)){
		sendData(t->type);
		sendData(t->owner);
		sendData(t->position);
	}
	if(checkMask(t,TOWER_TARGET) || checkMask(t,TOWER_CREATE)){
		short target=-1;
		if(t->target!=0)
			target=getGridId(t->target->position);
		sendData(target);
	}
	if(checkMask(t,TOWER_HEALTH) || checkMask(t,TOWER_CREATE))
		sendData(t->health);
	return 0;
}


int tickSendBullet(gnode* grid,bullet * b){
	int sock;
	sock=*((int*)grid);
	if (sock==0)
		return;
	char type=MSG_BULLET;
	sendData(type);
	sendData(b->id);
	sendData(b->bit_mask);
//	if(checkMask(b,BULLET_POSITION) || checkMask(b,BULLET_CREATE))
	sendData(b->position);
	if (checkMask(b,BULLET_CREATE)){
		sendData(b->type);
		sendData(b->owner);
		sendData(b->source);
//		sendData(b->destination);
	}
	if(checkMask(b,BULLET_DETONATE))
		sendData(b->detonate);
	return 0;
}

int sendPlayers(int sock,int player){
	int i;
	if (sock==0)
		return;
	for(i=0;i<=config.players_num;i++)
		if(i!=player){
			if(checkMask((&config.players[i]),PLAYER_HEALTH)){
				char mes=MSG_PLAYER;
				sendData(mes);
				sendData(i);
				sendData(config.players[i].base_health);
			}
		}
}

int sendTest(int sock){
	char mes=MSG_TEST;
	sendData(mes);
}
