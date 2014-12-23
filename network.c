#include "grid.h"
#include "file.h"
#include "gridmath.h"
#include "threads.h"
#include "network.h"
#include "engine.h"
#include "engine_tower.h"
#include "engine_npc.h"
#include "engine_bullet.h"
#include "types.h"

#define sendData(x) if(_sendData(sock,&x,sizeof(x))<=0) return -1

#define getSem(x) semget(IPC_PRIVATE, x, 0755 | IPC_CREAT)


int _sendData(int sock, void * buf, int size){
	int need=size;
	int get;
	get=send(sock,buf,need,MSG_NOSIGNAL);
	if (get<=0)
		return get;
	if (get==need)
		return get;
	printf("send not all\n");
	while(need>0){
		need-=get;
		if((get=send(sock,buf+(size-need),need,MSG_NOSIGNAL))<=0)
			return get;
	}
	return size;
}

int recvData(int sock, void * buf, int size){
	int need=size;
	int get;
	get=recv(sock,buf,need,0);
	if (get<=0)
		return -1;
	if (get==need)
		return get;
	printf("get not all\n");
	do{
		need-=get;
		if((get=recv(sock,buf+(size-need),need,0))<=0)
			return -1;
	}while(need>0);
	return size;
}

int processMessage(worker_arg * data,char type){
	struct sembuf sem_pl;
	memset(&sem_pl,0,sizeof(sem_pl));
	
	if (type==MSG_SPAWN_TOWER){
		int node_id=0;
		short t_id=0;
		tower_type *type;
		if(recvData(data->sock,&node_id,sizeof(node_id))<0){
			perror("recv Message");
			return -1;
		}
		if (recvData(data->sock,&t_id,sizeof(t_id))<0){
			perror("recv Message");
			return -1;
		}
		type=typesTowerGet(config.players[data->id].tower_set[t_id].id);
		if (type==0)
			return 0;
		if (config.players[data->id].money < type->cost) {//awesome
			printf("failed: not enough money\n");
			return 0;
		}
		printf("spawn tower %hd on %hd\n",t_id,node_id);		
		sem_pl.sem_num=0;
		sem_pl.sem_op=-1;
		semop(config.sem.player,&sem_pl,1);
		
		spawnTower(data->grid,node_id,data->id,config.players[data->id].tower_set[t_id].id);
		
		sem_pl.sem_num=0;
		sem_pl.sem_op=1;
		semop(config.sem.player,&sem_pl,1);
		return 0;
	}
	if (type==MSG_DROP_TOWER){
		int node_id=0;
		if(recvData(data->sock,&node_id,sizeof(node_id))<0){
			perror("recv Message");
			return -1;
		}
		printf("drop tower on %hd\n",node_id);		
		sem_pl.sem_num=0;
		sem_pl.sem_op=-1;
		semop(config.sem.player,&sem_pl,1);
		if (data->grid[node_id].tower!=0)
			removeTower(data->grid,data->grid[node_id].tower);
		sem_pl.sem_num=0;
		sem_pl.sem_op=1;
		semop(config.sem.player,&sem_pl,1);
		return 0;
	}
	if (type==MSG_SPAWN_NPC){
		int n_id=0;
		npc_type *type;
		if (recvData(data->sock,&n_id,sizeof(n_id))<0){
			perror("recv Message");
			return -1;
		}
		type=typesNpcGet(config.players[data->id].npc_set[n_id].id);
		if (type==0)
			return 0;
		if (config.players[data->id].money < type->cost) {//awesome
			printf("failed: not enough money\n");
			return 0;
		}		
		printf("spawn npc %d on %d\n",config.players[data->id].npc_set[n_id].id,config.points[config.bases[config.players[data->id].base_id].point_id].position);
		
		sem_pl.sem_num=0;
		sem_pl.sem_op=-1;
		semop(config.sem.player,&sem_pl,1);
		
		spawnNpc(data->grid,
				config.points[config.bases[config.players[data->id].base_id].point_id].position,
				data->id,
				config.players[data->id].npc_set[n_id].id);
		
		sem_pl.sem_num=0;
		sem_pl.sem_op=1;
		semop(config.sem.player,&sem_pl,1);
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
//	sem.sem_num=0;
//	sem.sem_op=1;
//	semop(config.sem.send,&sem,1);
	
	if ((config.sem.player=getSem(1))<0)
		perror("get semsend startServer");
//	sem.sem_num=0;
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
	
	if (checkMask(bit_mask,NPC_CREATE)){
		bit_mask|=NPC_LEVEL;
		bit_mask|=NPC_HEALTH;
		bit_mask|=NPC_SHIELD;
	}
	
	sendData(bit_mask);
	if (checkMask(bit_mask,NPC_CREATE)){
		sendData(n->owner);
		sendData(n->type);
	}
//	if(checkMask(bit_mask,NPC_POSITION) || checkMask(bit_mask,NPC_CREATE))
		sendData(n->position);
	if(checkMask(bit_mask,NPC_LEVEL))
		sendData(n->level);
	if(checkMask(bit_mask,NPC_HEALTH))
		sendData(n->health);
	if(checkMask(bit_mask,NPC_SHIELD))
		sendData(n->shield);
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
	if(checkMask(bit_mask,TOWER_CREATE)){
		bit_mask|=TOWER_TARGET;
		bit_mask|=TOWER_LEVEL;
		bit_mask|=TOWER_HEALTH;
		bit_mask|=TOWER_SHIELD;
	}
	sendData(bit_mask);
	
	if(checkMask(bit_mask,TOWER_CREATE)){
		sendData(t->type);
		sendData(t->owner);
		sendData(t->position);
	}
	if(checkMask(bit_mask,TOWER_TARGET)){
		short target=-1;
		if(t->target!=0)
			target=getGridId(t->target->position);
		sendData(target);
	}
	if(checkMask(bit_mask,TOWER_LEVEL)){
		sendData(t->level);
	}
	if(checkMask(bit_mask,TOWER_HEALTH))
		sendData(t->health);
	if(checkMask(bit_mask,TOWER_SHIELD))
		sendData(t->shield);
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
	if (checkMask(bit_mask,BULLET_CREATE)){
	}
	
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

int sendPlayers(int sock,int id){
	int i;
	int bit_mask;
	char mes;
	if (sock==0)
		return 0;
	for(i=0;i<=config.players_num;i++){
		bit_mask=config.players[i].bit_mask;
		if (config.players[id].first_send!=0){
			bit_mask|=PLAYER_CREATE;
			bit_mask|=PLAYER_MONEY;
		}
		if (id != i)
			bit_mask &= ~PLAYER_MONEY;
		mes=MSG_PLAYER;
		sendData(mes);
		sendData(i);
		sendData(bit_mask);
		if(checkMask(bit_mask,PLAYER_CREATE)){
			sendData(config.players[i].id);
			sendData(config.players[i].tower_set);
			sendData(config.players[i].npc_set);
			sendData(config.players[i].group);
			//send info about base
			sendData(config.players[i].base_type.health);
			int base_id=0;
			if (config.players[i].base!=0) //TODO: check why not on first time
				base_id=config.players[i].base->id;
			sendData(base_id);
		}	
		if(checkMask(bit_mask,PLAYER_HEALTH) || checkMask(bit_mask,PLAYER_CREATE))
			sendData(config.players[i].base_type.health);
		if(checkMask(bit_mask,PLAYER_LEVEL))
			sendData(config.players[i].level);
		if(/*i == id && */(checkMask(bit_mask,PLAYER_MONEY)))
			sendData(config.players[i].money);
	}
	return 0;
}

int sendTest(int sock){
	char mes=MSG_TEST;
	sendData(mes);
	return 0;
}

int networkAuth(worker_arg *data){
	int sock=data->sock;
	int tmp;
	sendData(data->id);
	//latency check
	recvData(sock,&tmp,sizeof(tmp));
	sendData(tmp);
	return 0;
}
