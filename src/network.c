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


#define sendData(x) if(({typeof(x) _x=biteSwap(x);_sendData(sock,&_x,sizeof(_x));})<=0) return -1

#define getSem(x) t_semget(IPC_PRIVATE, x, 0755 | IPC_CREAT)


int _sendData(int sock, void * buf, int size){
	int need=size;
	int get;
	get=send(sock,buf,need,MSG_NOSIGNAL);
	if (get<=0)
		return get;
	if (get==need)
		return get;
	printDebug("send not all\n");
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
	get=recv(sock,buf,need,MSG_DONTWAIT);
	if (get==0)
		return 0;
	if (get<0)
		if (errno!=EAGAIN)
			return -1;
	if (get==need)
		return get;
	printDebug("get not all\n");
	int $$=0;
	do{
		printDebug("try %d to read %d\n", $$, need);
		if (get>0)
			need-=get;
//		printf("try to get\n");
		if((get=recv(sock,buf+(size-need),need,MSG_DONTWAIT))<=0)
			if (errno!=EAGAIN)
				return -1;
		usleep(80000);//80ms*18 ~ 1440ms waiting
		$$++;
		if ($$>18)//max tries of read
			return -1;
	}while(need>0);
	return size;
}

//functions for work with packets

//create new packet
packet* packetNew(int sock){
	packet * p;
	if ((p=malloc(sizeof(*p)))==0)
		return 0;
	memset(p,0,sizeof(*p));
	p->sock=sock;
	return p;
}

#define packetAddData(data) ({typeof(data) a=biteSwap(data);packetAdd(pack, &a, sizeof(a));})
//add data to packet, if size of packet more than PACKET_SIZE, send it and start new
int packetAdd(packet *p, void* data, int size){
	int o=1;
	if (p->size+size>PACKET_SIZE){
		o=_sendData(p->sock, p->buf, p->size);
		p->size=0;
	}
	memcpy(p->buf+p->size,data,size);
	p->size+=size;
	return o;
}

//send packet and free memory
int packetFinish(packet *p){
	int o=_sendData(p->sock, p->buf, p->size);
	free(p);
	return o;
}

//massages

int processMessage(worker_arg * data,char type){
	struct sembuf sem_pl;
	memset(&sem_pl,0,sizeof(sem_pl));
	
	//player not fail and game started
	int playerNotFailed = config.players[data->id].base == 0 || config.game.wait_start>0 ? 0 : 1;
	
	if (type==MSG_SPAWN_TOWER){
		int node_id=0;
		short t_id=0;
		tower_type *type;
		if(recvData(data->sock,&node_id,sizeof(node_id))<0){
			perror("recv Message");
			return -1;
		}
		node_id=biteSwap(node_id);
		if (recvData(data->sock,&t_id,sizeof(t_id))<0){
			perror("recv Message");
			return -1;
		}
		t_id=biteSwap(t_id);
		if (playerNotFailed) {
			type=typesTowerGet(config.players[data->id].tower_set[t_id].id);
			if (type==0)
				return 0;
			if (config.players[data->id].money < type->cost) {//awesome
				printDebug("Player %d failed: not enough money\n", data->id);
				return 0;
			}
			printDebug("try %d spawn tower %hd on %hd\n",data->id,t_id,node_id);		
			if (data->grid[node_id].buildable>0){
				sem_pl.sem_num=0;
				sem_pl.sem_op=-1;
				t_semop(t_sem.player,&sem_pl,1);
			
				spawnTower(data->grid,node_id,data->id,config.players[data->id].tower_set[t_id].id);
			
				sem_pl.sem_num=0;
				sem_pl.sem_op=1;
				t_semop(t_sem.player,&sem_pl,1);
				printDebug("spawned\n");
			}
		}
		return 0;
	}
	if (type==MSG_DROP_TOWER){
		int node_id=0;
		if(recvData(data->sock,&node_id,sizeof(node_id))<0){
			perror("recv Message");
			return -1;
		}
		node_id=biteSwap(node_id);
		if (playerNotFailed) {
			printDebug("%d drop tower on %hd\n",data->id,node_id);		
			sem_pl.sem_num=0;
			sem_pl.sem_op=-1;
			t_semop(t_sem.player,&sem_pl,1);
			if (data->grid[node_id].tower!=0)
				removeTower(data->grid,data->grid[node_id].tower);
			sem_pl.sem_num=0;
			sem_pl.sem_op=1;
			t_semop(t_sem.player,&sem_pl,1);
		}
		return 0;
	}
	if (type==MSG_SPAWN_NPC){
		int n_id=0;
		npc_type *type;
		if (recvData(data->sock,&n_id,sizeof(n_id))<0){
			perror("recv Message");
			return -1;
		}
		n_id=biteSwap(n_id);
		if (playerNotFailed) {
			type=typesNpcGet(config.players[data->id].npc_set[n_id].id);
			if (type==0)
				return 0;
			if (config.players[data->id].money < type->cost) {//awesome
				printDebug("Player %d failed: not enough money\n", data->id);
				return 0;
			}		
			printDebug("%d spawn npc %d on %d\n",data->id,config.players[data->id].npc_set[n_id].id,config.points[config.bases[config.players[data->id].base_id].point_id].position);
			
			sem_pl.sem_num=0;
			sem_pl.sem_op=-1;
			t_semop(t_sem.player,&sem_pl,1);
			
			spawnNpc(data->grid,
					config.points[config.bases[config.players[data->id].base_id].point_id].position,
					data->id,
					config.players[data->id].npc_set[n_id].id);
			
			sem_pl.sem_num=0;
			sem_pl.sem_op=1;
			t_semop(t_sem.player,&sem_pl,1);
		}
		return 0;
	}
	if (type==MSG_MOVE_NPCS){
		int i, id;
		int node=0;
		char num;
		npc * h=0;
		if (recvData(data->sock,&node,sizeof(node))<0){
			perror("recv Message");
			return -1;
		}
		node=biteSwap(node);
		if (recvData(data->sock,&num,sizeof(num))<0){
			perror("recv Message");
			return -1;
		}
		num=biteSwap(num);
		for (i=0;i<num;i++){
			if (recvData(data->sock,&id,sizeof(id))<0){
				perror("recv Message");
				return -1;
			}
			id=biteSwap(id);
			if (playerNotFailed) {
				h=getNpcById(id);
				if (h!=0){
					printDebug("%d move npc %d to %d\n",data->id, id, node);
					//move hero to node
					setNpcTargetByNode(data->grid,h,node);
				}
			}
		}
		return 0;
	}
	if (type==MSG_SET_TARGET){
		char type;
		if (recvData(data->sock,&type,sizeof(type))<0){
			perror("recv Message");
			return -1;
		}
		type=biteSwap(type);
		if (playerNotFailed) {
			config.players[data->id].target=type;
			config.players[data->id].target_changed=1;
			setMask(&config.players[data->id],PLAYER_TARGET);
		}
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
	if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0){
		perror("bind startServer");
		exit(1);
	}
	
	if(listen(listener, 1)<0)
		perror("listen startServer");
	
	if ((t_sem.send=getSem(3))==0)
		perror("get semsend startServer");
//	sem.sem_num=0;
//	sem.sem_op=1;
//	t_semop(t_sem.send,&sem,1);
	
	if ((t_sem.player=getSem(1))==0)
		perror("get semsend startServer");
//	sem.sem_num=0;
	sem.sem_op=1;
	t_semop(t_sem.player,&sem,1);
	
	config.game.run=1;
	if (startListener(listener,grid)<=0)
		perror("startListener");
	
	return listener;
}

int realizeServer(){
	int o=0;
	if (t_semctl(t_sem.send,0,IPC_RMID)<0){
		perror("t_semctl send realizeServer");
		o++;
	}
	if (t_semctl(t_sem.player,0,IPC_RMID)<0){
		perror("t_semctl player realizeServer");
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
	packet* pack=packetNew(sock);
	
	int bit_mask=n->bit_mask;
	bit_mask|=NPC_POSITION;
	
	if (config.players[id].first_send!=0)
		bit_mask|=NPC_CREATE;
	
	if (checkMask(bit_mask,NPC_CREATE)){
		bit_mask|=NPC_LEVEL;
		bit_mask|=NPC_HEALTH;
		bit_mask|=NPC_SHIELD;
		bit_mask|=NPC_POSITION;
		bit_mask|=NPC_STATUS;
	}
	if (bit_mask==0)
		return 0;
	
	char type=MSG_NPC;
	
	packetAddData(type);//sendData(type);
	packetAddData(bit_mask);//sendData(bit_mask);
	packetAddData(n->id);//sendData(n->id);
	
	if (checkMask(bit_mask,NPC_CREATE)){
		packetAddData(n->owner);//sendData(n->owner);
		packetAddData(n->type);//sendData(n->type);
	}
	if(checkMask(bit_mask,NPC_POSITION))
		packetAddData(n->position.x);//sendData(n->position.x);
		packetAddData(n->position.y);//sendData(n->position.y);
	if(checkMask(bit_mask,NPC_LEVEL))
		packetAddData(n->level);//sendData(n->level);
	if(checkMask(bit_mask,NPC_HEALTH))
		packetAddData(n->health);//sendData(n->health);
	if(checkMask(bit_mask,NPC_SHIELD))
		packetAddData(n->shield);//sendData(n->shield);
	if(checkMask(bit_mask,NPC_STATUS))
		packetAddData(n->status);//sendData(n->status); //byte
	
	return (packetFinish(pack)<=0) ? -1 : 0;
}


int tickSendTower(gnode* grid,tower* t){
	int sock,id;
	sock=((worker_arg*)grid)->sock;
	id=((worker_arg*)grid)->id;
	if (sock==0)
		return 0;
	packet* pack=packetNew(sock);
	
	int bit_mask=t->bit_mask;
	if (config.players[id].first_send!=0)
		bit_mask|=TOWER_CREATE;
	if(checkMask(bit_mask,TOWER_CREATE)){
		bit_mask|=TOWER_TARGET;
		bit_mask|=TOWER_LEVEL;
		bit_mask|=TOWER_HEALTH;
		bit_mask|=TOWER_SHIELD;
	}
	if (bit_mask==0)
		return 0;
	
	char type=MSG_TOWER;
	
	packetAddData(type);
	packetAddData(bit_mask);
	packetAddData(t->id);
	
	if(checkMask(bit_mask,TOWER_CREATE)){
		packetAddData(t->type);
		packetAddData(t->owner);
		packetAddData(t->position);
	}
	if(checkMask(bit_mask,TOWER_TARGET)){
		short target=-1;
		if(t->target!=0)
			target=getGridId(t->target->position);
		packetAddData(target);
	}
	if(checkMask(bit_mask,TOWER_LEVEL)){
		packetAddData(t->level);
	}
	if(checkMask(bit_mask,TOWER_HEALTH))
		packetAddData(t->health);
	if(checkMask(bit_mask,TOWER_SHIELD))
		packetAddData(t->shield);
	
	return (packetFinish(pack)<=0) ? -1 : 0;
}


int tickSendBullet(gnode* grid,bullet * b){
	int sock,id;
	sock=((worker_arg*)grid)->sock;
	id=((worker_arg*)grid)->id;
	if (sock==0)
		return 0;
	packet* pack=packetNew(sock);
	
	int bit_mask=b->bit_mask;
	if (config.players[id].first_send!=0){
		bit_mask|=BULLET_CREATE;
	}
	if (checkMask(bit_mask,BULLET_CREATE)){
		bit_mask|=BULLET_POSITION;
	}
	bit_mask|=BULLET_POSITION;
//	if (bit_mask==0)
//		return 0;
	
	char type=MSG_BULLET;
	
	packetAddData(type);	
	packetAddData(bit_mask);
	packetAddData(b->id);
	
	if(checkMask(bit_mask,BULLET_POSITION)){
		packetAddData(b->position.x);
		packetAddData(b->position.y);
	}
	if (checkMask(bit_mask,BULLET_CREATE)){
		packetAddData(b->type);
		packetAddData(b->owner);
		packetAddData(b->source.x);
		packetAddData(b->source.y);
//		packetAddData(b->destination);
	}
	if(checkMask(bit_mask,BULLET_DETONATE))
		packetAddData(b->detonate);
	
	return (packetFinish(pack)<=0) ? -1 : 0;
}

int sendPlayers(int sock,int id){
	int i;
	int bit_mask;
	char mes;
	if (sock==0)
		return 0;

	packet* pack=packetNew(sock);	
	for(i=0;i<=config.game.players;i++){		
		if (config.players[i].id==0)
			continue;
		bit_mask=config.players[i].bit_mask;
		if (config.players[id].first_send!=0 || config.players[i].first_send!=0){
			bit_mask|=PLAYER_CREATE;
		}
		if(checkMask(bit_mask,PLAYER_CREATE)){
			bit_mask|=PLAYER_SETS;
			bit_mask|=PLAYER_MONEY;
			bit_mask|=PLAYER_HERO;
			bit_mask|=PLAYER_HERO_COUNTER;
			bit_mask|=PLAYER_BASE;
			bit_mask|=PLAYER_TARGET;
		}
		if (id != i){
			bit_mask &= ~PLAYER_MONEY;
			bit_mask &= ~PLAYER_TARGET;
		}
		mes=MSG_PLAYER;
		
		if (bit_mask==0)
			continue;
		
		packetAddData(mes);
		packetAddData(bit_mask);
		packetAddData(i);
		int j;
		if(checkMask(bit_mask,PLAYER_CREATE)){
			packetAddData(config.players[i].id);
			packetAddData(config.players[i].group);
			packetAddData(config.players[i]._hero_counter);
			//base type
			packetAddData(config.players[i].base_type.health);
			//hero type
			packetAddData(config.players[i].hero_type.health);
			packetAddData(config.players[i].hero_type.shield);
		}
		if(checkMask(bit_mask,PLAYER_SETS)){
			for(j=0;j<TOWER_SET_NUM;j++){
				packetAddData(config.players[i].tower_set[j].id);
				packetAddData(config.players[i].tower_set[j].num);
			}
			for(j=0;j<NPC_SET_NUM;j++){
				packetAddData(config.players[i].npc_set[j].id);
				packetAddData(config.players[i].npc_set[j].num);
			}
		}
		if(checkMask(bit_mask,PLAYER_HERO)){
			int hero_id=0;
			if (config.players[i].hero!=0) //TODO: check why not on first time
				hero_id=config.players[i].hero->id;
			packetAddData(hero_id);
		}
		if(checkMask(bit_mask,PLAYER_HERO_COUNTER)){
			packetAddData(config.players[i].hero_counter);
		}
		if(checkMask(bit_mask,PLAYER_BASE)){
			int base_id=0;
			if (config.players[i].base!=0) //TODO: check why not on first time
				base_id=config.players[i].base->id;
			packetAddData(base_id);
		}
		if(checkMask(bit_mask,PLAYER_LEVEL))
			packetAddData(config.players[i].level);
		if(/*i == id && */(checkMask(bit_mask,PLAYER_MONEY))){
			printDebug("send_money\n");
			packetAddData(config.players[i].money);
		}
		if(checkMask(bit_mask,PLAYER_TARGET)){
			packetAddData(config.players[i].target);
		}
		if(checkMask(bit_mask,PLAYER_FAIL)) { //TODO: decomment and add to client app
			packetAddData(config.players[i].stat.xp);
		}
	}
	packetFinish(pack);
	
//	return (packetFinish(pack)<=0) ? -1 : 0;
	
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
	sendData(config.game.players);
	//latency check
	recvData(sock,&tmp,sizeof(tmp));
	sendData(tmp);
	return 0;
}

int networkWaitingTime(worker_arg *data){
	int sock=data->sock;
	int wait=config.game.wait_start/1000; //miliseconds
	char msg=MSG_INFO;
	int mes=MSG_INFO_WAITING_TIME;
	sendData(msg);
	sendData(mes);
	sendData(wait); 
//	printDebug("send time wait\n");
	return 0;
}

int connectToHost(char* host, int port){
	int sockfd;
	struct sockaddr_in servaddr;
	struct hostent *server;
	server = gethostbyname(host);
	if (server == NULL) {
		perror("gethostbyname");
		return -1;
	}
	
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
		perror("socket");
		return -1;
	}
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	memcpy((char *)&servaddr.sin_addr.s_addr,(char *)server->h_addr, server->h_length);
//	servaddr.sin_addr.s_addr=inet_addr("172.16.1.40");//argv[1]);
	servaddr.sin_port=htons(port);

	if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))<0){
		perror("connect");
		return -2;
	}	
	return sockfd;

}


int networkPortTake(){
	int manager=0;
	char $_$=0;
	manager=connectToHost("localhost",7920);
	if (manager==0)
		return -1;
	printDebug("connected to manager\n");
	if (_sendData(manager,&config.game.port,sizeof(config.game.port))<=0)
		return -1;
	printDebug("sent port\n");
	if (recvData(manager,&$_$,sizeof($_$))<=0)
		return -1;
	printDebug("get %d\n",$_$);
	if ($_$!=-1)
		return -1;
	$_$=1;
	if (_sendData(manager,&$_$,sizeof($_$))<=0)
		return -1;
	printDebug("send ");
	close(manager);
	return 0;
}

int networkPortFree(){
	if (config.game.token!=0){
		int manager=0;
		char $_$=0;
		manager=connectToHost("localhost",7920);
		if (manager!=0){
			if (_sendData(manager,&config.game.port,sizeof(config.game.port))<=0)
				return -1;
			if (_sendData(manager,&$_$,sizeof($_$))<=0)
				return -1;
			close(manager);
		}
	}
	return 0;
}

int wrongByteOrder(){
	char c4[4]={-92, 112, 69, 65};
	float *f=(void*)c4, f0=12.34;
	int *i=(void*)c4, i0=1095069860;
	if (biteSwap(*i)!=i0){
		perror("wrongByteOrder int");
		return 1;
	}
	if (biteSwap(*f)!=f0){
		perror("wrongByteOrder float");
		return 1;
	}
	char c8[8]={-82, 71, -31, 122, 20, -82, 40, 64};
	long long *l=(void*)c8, l0=4623136420479977390;
	double *d=(void*)c8, d0=12.34;
	if (biteSwap(*l)!=l0){
		perror("wrongByteOrder long long");
		return 1;
	}
	if (biteSwap(*d)!=d0){
		perror("wrongByteOrder double");
		return 1;
	}
	return 0;
}

