#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "grid.h"
#include "file.h"
#include "gridmath.h"

#define sendData(x) send(sock,&x,sizeof(x),0)


int startServer(int players, int port){
	int sock, listener;
	struct sockaddr_in addr;
	int connected=0;
	if((listener = socket(AF_INET, SOCK_STREAM, 0))<0)
		perror("socket startServer");
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		perror("bind startServer");
	
	if(listen(listener, 1)<0)
		perror("listen startServer");
	
	while(connected<players){
		if((sock = accept(listener, NULL, NULL))<0)
			perror("accept startServer");
		//check connected user
		
		//need to change later
		return sock;
	}
	return 0;
}


void tickSendNpc(gnode* grid,npc* n){
	int sock;
	sock=*((int*)grid);
	char type=MSG_NPC;
	sendData(type);
	sendData(n->id);
	sendData(n->bit_mask);
	if (!n->bit_mask){
		sendData(n->isfriend);
		sendData(n->type);
	}
	if(checkMask(n,NPC_POSITION) || !n->bit_mask)
		sendData(n->position);
	if(checkMask(n,NPC_HEALTH) || !n->bit_mask)
		sendData(n->health);
}

void tickSendTower(gnode* grid,tower* t){
	int sock;
	sock=*((int*)grid);
	char type=MSG_TOWER;
	sendData(type);
	sendData(t->id);
	sendData(t->bit_mask);
	if(!t->bit_mask){
		sendData(t->type);
		sendData(t->owner);
		sendData(t->position);
	}
	if(checkMask(t,TOWER_TARGET) || !t->bit_mask){
		int target=-1;
		if(t->target!=0)
			target=getGridId(t->target->position);
		sendData(target);
	}
	if(checkMask(t,TOWER_HEALTH) || !t->bit_mask)
		sendData(t->health);
}


void tickSendBullet(gnode* grid,bullet * b){
	int sock;
	sock=*((int*)grid);
	char type=MSG_BULLET;
	sendData(type);
	sendData(b->id);
	sendData(b->bit_mask);
	if (!b->bit_mask){
		sendData(b->type);
		sendData(b->owner);
	}
	if(checkMask(b,BULLET_POSITION) || !b->bit_mask)
		sendData(b->position);
	if(checkMask(b,BULLET_DETONATE))
		sendData(b->detonate);
}

void sendPlayers(int sock,int player){
	int i;
	for(i=0;i<config.players_num;i++)
		if(i!=player){
			if(checkMask((&config.players[i]),PLAYER_HEALTH)){
				char mes=MSG_PLAYER;
				sendData(mes);
				sendData(i);
				sendData(config.players[i].base_health);
			}
		}
}
