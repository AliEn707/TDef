#include "grid.h"
#include "engine.h"


int timePassed(int i){
	//config.time  struct timeval
	struct timeval end;
	gettimeofday(&end, NULL);
	int out=((end.tv_sec - config.time.tv_sec)*1000000+
			end.tv_usec - config.time.tv_usec);
	if(i>0)
		if (out>1000000/TPS)
			perror("time to tick");
	memcpy(&config.time,&end,sizeof(end));
	return out;
}


void syncTPS(){
	int z=timePassed(1);
	int must=1000;
	if((z=(1000000/TPS-must)-z)>0){
		usleep(z);
	}
	usleep(must);
	timePassed(0);
}


void initArrays(){
	if ((config.tower_array=malloc(sizeof(tower)*config.tower_max))==0)
		perror("malloc tower initArrays");
	memset(config.tower_array,0,sizeof(tower)*config.tower_max);
	if ((config.npc_array=malloc(sizeof(npc)*config.npc_max))==0)
		perror("malloc NPC initArrays");
	memset(config.npc_array,0,sizeof(npc)*config.npc_max);
	if ((config.bullet_array=malloc(sizeof(bullet)*config.bullet_max))==0)
		perror("malloc bullet initArrays");
	memset(config.bullet_array,0,sizeof(bullet)*config.bullet_max);
	if ((config.players=malloc(sizeof(bullet)*config.player_max))==0)
		perror("malloc player initArrays");
	memset(config.players,0,sizeof(bullet)*config.player_max);
	initAreaArray();
//	printf("%d %d %d\n",sizeof(tower)*config.tower_max,sizeof(npc)*config.npc_max,sizeof(bullet)*config.bullet_max);
}

void realizeArrays(){
	free(config.players);
	free(config.tower_array);
	free(config.npc_array);
	free(config.bullet_array);
	realizeAreaArray();
}


npc* newNpc(){
	int i;
	for(i=0;i<config.npc_max;i++)
		if (config.npc_array[i].id==0){
			config.npc_array[i].id=getGlobalId();
			return &config.npc_array[i];
		}	
	return 0;
}


npc* addNpc(gnode* node,npc* n){
	npc **root=n->isfriend==1?&node->fnpcs:&node->enpcs;
	if (*root==0){
		*root=n;
		return n;
	}else{
		npc* tmp;
		for(tmp=*root;tmp->next!=0;tmp=tmp->next);
		tmp->next=n;
		n->next=0;
		return n;
	}
	return 0;
}


npc*  getNpc(gnode* grid,npc* n){
	npc* tmp;
	gnode * node=&grid[getGridId(n->position)];
	npc **root=n->isfriend?&node->fnpcs:&node->enpcs;
	if (*root!=0){
		tmp=*root;
		if (*root==n){
			*root=n->next;
		}else{
			for(tmp=*root;tmp->next!=n;tmp=tmp->next)
				if (tmp->next==0)
					return 0;
//			npc* out=tmp->next;
			tmp->next=tmp->next->next;
		}
		n->next=0;
		return n;
	}
	return 0;
}

int delNpc(gnode* grid,npc* n){
	npc* tmp=getNpc(grid,n);
	if (tmp!=0){
		memset(tmp,0,sizeof(npc));
		return 0;
	}
	return -1;
}

void setNpcBase(npc* n){
	n->health=config.npc_types[n->type].health;
	n->shield=config.npc_types[n->type].shield;
	//may be more
}


npc* spawnNpc(gnode* grid,int node_id,int isfriend,int type){
	npc* n;
	if((n=newNpc())==0)
		perror("newNpc spawnNpc");
	n->isfriend=isfriend;
	n->status=IN_MOVE;
	n->position.x=getGridx(node_id);
	n->position.y=getGridy(node_id);
	n->path_count=NPC_PATH;
	memcpy(&n->destination,&n->position,sizeof(vec));
	n->type=type;
	setNpcBase(n);
	if (addNpc(&grid[node_id],n)==0)
		perror("addMpc spawnNpc");
	return n;
}

int canSee(gnode* grid,vec* a,vec* b){
	float x1=a->x;
	float y1=a->y;
	float x2=b->x;
	float y2=b->y;
	int destination=to2d((int)x2,(int)y2);
//	printf("%g %g",x2,y2);
	if (x1!=x2){
		if (x1>x2){
			int tmp;
			tmp=x1;
			x1=x2;
			x2=tmp;
			tmp=y1;
			y1=y2;
			y2=tmp;
		}
	
		float K=(y2-y1)/(x2-x1);
		float B=(y1*(x2-x1)-x1*(y2-y1))/(x2-x1);
		 
		for(;x1<x2;x1+=0.3){
			y1=K*x1+B;
			if (to2d(((int)x1),((int)y1))!=destination){
				if (grid[to2d(((int)x1),((int)y1))].walkable<0||
					grid[to2d(((int)x1),((int)y1))].tower>0){
					return -1;
				}
			}
		}
	}else{
		if (y1>y2){
			int tmp;
			tmp=x1;
			x1=x2;
			x2=tmp;
			tmp=y1;
			y1=y2;
			y2=tmp;
		}
	
		float K=(x2-x1)/(y2-y1);
		float B=(x1*(y2-y1)-y1*(x2-x1))/(y2-y1);
		 
		for(;y1<y2;y1+=0.3){
			x1=K*y1+B;
			if (to2d(((int)x1),((int)y1))!=destination){
				if (grid[to2d(((int)x1),((int)y1))].walkable<0||
					grid[to2d(((int)x1),((int)y1))].tower>0){
					return -1;
				}
			}
		}
	}
	return 1;
}

int canWalkThrough(gnode* grid,vec* a,vec* b){
	float x1=a->x;
	float y1=a->y;
	float x2=b->x;
	float y2=b->y;
	int destination=to2d((int)x2,(int)y2);
	if (x1!=x2){
		if (x1>x2){
			int tmp;
			tmp=x1;
			x1=x2;
			x2=tmp;
			tmp=y1;
			y1=y2;
			y2=tmp;
		}
		float K=(y2-y1)/(x2-x1);
		float B=(y1*(x2-x1)-x1*(y2-y1))/(x2-x1);
		
		for(;x1<=x2;x1+=0.3){
			y1=K*x1+B;
			if (to2d(((int)x1),((int)y1))!=destination){
				if (grid[to2d(((int)x1),((int)y1))].walkable<=0||
					grid[to2d(((int)x1),((int)y1))].tower>0){
					return -1;
				}
			}
		}
	}else{
		if (y1>y2){
			int tmp;
			tmp=x1;
			x1=x2;
			x2=tmp;
			tmp=y1;
			y1=y2;
			y2=tmp;
		}
		float K=(x2-x1)/(y2-y1);
		float B=(x1*(y2-y1)-y1*(x2-x1))/(y2-y1);
		 
		for(;y1<=y2;y1+=0.3){
			x1=K*y1+B;
			if (to2d(((int)x1),((int)y1))!=destination){
				if (grid[to2d(((int)x1),((int)y1))].walkable<=0||
					grid[to2d(((int)x1),((int)y1))].tower>0){
					return -1;
				}
			}
		}
	}
	return 1;
}




tower* findNearTower(gnode* grid,npc* n,int range){
	/**/
	int i,j,k;
	int x=(int)n->position.x;
	int y=(int)n->position.y;
	int yid,xid;
	printf("%d\n",n->id);
	for(i=0;i<range;i++){
		for(j=0;j<config.area_size[i];j++)
			if (((xid=x+config.area_array[i][j].x)>=0 && x+config.area_array[i][j].x<config.gridsize) &&
					((yid=y+config.area_array[i][j].y)>=0 && y+config.area_array[i][j].y<config.gridsize))
				if (grid[to2d(xid,yid)].tower!=0)
					if (canSee(grid,&(vec){n->position.x,n->position.y},&(vec){xid+0.5,yid+0.5})>0) //can see check
						if(config.players[grid[to2d(xid,yid)].tower->owner].isfriend!=n->isfriend)
							if(canWalkThrough(grid,&(vec){n->position.x,n->position.y},&(vec){xid+0.5,yid+0.5})>0 || rand()%100<30){//can walk check or rand<30%
								n->ttarget=grid[to2d(xid,yid)].tower;
//								printf("? %d\n",to2d(xid,yid));
								if(rand()%100<40)
									return n->ttarget;
							}
		if(n->ttarget!=0)
			return n->ttarget;
	}
//	config.area_array
//	config.tower_types[i].see_distanse
	return n->ttarget;
}

npc* diedCheckNpc(npc* n){
	return n;
}


int findEnemyBase(int isfriend){
	#define t config.tower_array
	int i;
	int id=-1;
	for(i=0;i<config.tower_max;i++)
		if (t[i].id>0)
			if (t[i].type==BASE)
				if (config.players[t[i].owner].isfriend!=isfriend){//add friend check
					id=t[i].position;
					if (rand()%100<40)
						return id;
				}
	return id;
	#undef t
}


void tickTargetNpc(gnode* grid,npc* n){
	if (n->status!=IN_ATTACK){
		if(n->ttarget==0)
			if (findNearTower(grid,n,config.npc_types[n->type].see_distanse)!=0)
				return;
		//if near no Towers
		int id;
		if((id=findEnemyBase((int)n->isfriend))<0){
			perror("findEnemyBase tickTargetNpc");
			return;
		}
		if ((n->ttarget=grid[id].tower)==0)
			perror("ttarget tickTargetNpc");
	}
}


void tickAttackNpc(gnode* grid,npc* n){
	if (n->status==IN_ATTACK){
		//if target !=0
		//-attacking
		//else set IN_MOVE
		//???????
//		printf("\t%d %d\n",n->attack_count,config.npc_types[n->type].attack_speed);
		if (n->ttarget!=0)
			if (n->attack_count>=config.npc_types[n->type].attack_speed){
				n->attack_count=0;
				bullet* b;//set params of bullet
				if ((b=newBullet())==0){
					perror("newBullet tickAttackBullet");
					return;
				}
				memcpy(&b->position,&n->position,sizeof(vec));
				memcpy(&b->source,&n->position,sizeof(vec));
				b->destination.x=getGridx(n->ttarget->position);
				b->destination.y=getGridy(n->ttarget->position);
				b->type=config.npc_types[n->type].bullet_type;
				b->damage=config.npc_types[n->type].damage;
//				memcpy(&b->effects,&config.npc_types[n->type].effects,sizeof(effects));
			}
	}else{
		//search target in attack distanse
		//if finded set IN_ATTACK
		tower* tmp=n->ttarget;
		n->ttarget=0;
		if (findNearTower(grid,n,config.npc_types[n->type].attack_distanse)!=0){
			n->status=IN_ATTACK;
//			printf("%d\n",config.npc_types[n->type].attack_distanse);
			return;
		}
		n->ttarget=tmp;
	}
}
  
void tickDiedCheckNpc(gnode* grid,npc* n){
	n->ttarget=diedCheckTower(n->ttarget);
}



/*
must be this
tickMiscNpc
tickMiscTower
tickMiscBullet
tickDiedCheckNpc
tickDiedCheckTower
tickCleanNpc
tickCleanTower
tickCleanBullet
tickTargetNpc
tickAtackNpc
tickAtackTower
tickProcessBullet
tickMoveNpc

some user stuff
-remove tower set it to died

*/


void tickCleanNpc(gnode* grid,npc* n){
	if (n->health<0)
		memset(n,0,sizeof(npc));
	//foeachNpc
}

void tickMoveNpc(gnode* grid,npc* n){
	if (n->status==IN_MOVE){
		if (n->ttarget!=0){
			//check path from position
			if (eqInD(n->position.x,n->destination.x,config.npc_types[n->type].move_speed) &&
						eqInD(n->position.y,n->destination.y,config.npc_types[n->type].move_speed)||
				glength(&n->position,&n->destination)<0.05){
				if (n->path_count>=NPC_PATH){
					memset(n->path,-1,sizeof(int)*NPC_PATH);
					if(aSearch(grid,
							grid+n->ttarget->position,
							grid+getGridId(n->position),
							n->path)<0)
						perror("aSearch tickMoveNpc");
					n->path_count=0;
					}
				
				int node_id;
				node_id=n->path[n->path_count++];
				n->destination.x=getGridx(node_id);
				n->destination.y=getGridy(node_id);
			}
			
			vec dir={0,0};
			getDir(&n->position,&n->destination,&dir);
			
			vec pos={n->position.x+dir.x*config.npc_types[n->type].move_speed,
					n->position.y+dir.y*config.npc_types[n->type].move_speed};
			//check node change 
			int a,b;
			if ((a=getGridId(n->position))!=(b=getGridId(pos))){
				npc* tn;
				if((tn=getNpc(grid,n))==0)
					perror("getNpc tickMoveNpc");
				else
					addNpc(&grid[b],tn);
			}
			
			//write new position
			memcpy(&n->position,&pos,sizeof(vec));
		}
	}
}

void tickMiscNpc(gnode* grid,npc* n){
	if (n->attack_count<config.npc_types[n->type].attack_speed)
		n->attack_count++;
	n->bit_mask=0;
}


void forEachNpc(gnode* grid, void (process)(gnode*g,npc*n)){//add function
	int i;
	for(i=0;i<config.npc_max;i++)
		if(config.npc_array[i].id>0)
			process(grid,&config.npc_array[i]);
}

//////////////towers

tower* newTower(){
	int i;
	for(i=0;i<config.tower_max;i++)
		if (config.tower_array[i].id==0){
			config.tower_array[i].id=getGlobalId();
			return &config.tower_array[i];
		}	
	return 0;
}



tower* diedCheckTower(tower* n){
	if (n==0)
		return 0;
	if (n->type!=BASE)
		if (n->health<=0)
			return 0;
	return n;
}


void setTowerBase(tower* t){
	if (t->type==BASE){
		t->health=config.players[t->owner].base_health;
//		t->energy=config.players[t->owner].base_energy;
	}
	else{
		t->health=config.tower_types[t->type].health;
		t->energy=config.tower_types[t->type].energy;
	}
}


tower* spawnTower(gnode * grid,int node_id,int owner,int type){
	tower* t;
	gnode* node=&grid[node_id];
	if((t=newTower())==0)
		perror("newTower spawnTower");
	t->owner=owner;
	t->position=node_id;
	t->type=type;
	setTowerBase(t);
	node->tower=t;
	return t;
}

int delTower(tower* t){
	
}

int removeTower(gnode * grid,tower* t){
	gnode * node=&grid[t->position];
	node->tower=0;
	t->health=-1;
	return 0;
}

void tickMiscTower(gnode* grid,tower* t){
	if (t->attack_count<config.tower_types[t->type].attack_speed)
		t->attack_count++;
	t->bit_mask=0;
}

void tickDiedCheckTower(gnode* grid,tower* t){
	/**/
}

void tickAttackTower(gnode* grid,tower* t){
	/**/
}

void tickCleanTower(gnode* grid,tower* t){
	/**/
}



void forEachTower(gnode* grid, void (process)(gnode*g,tower*t)){//add function
	int i;
	for(i=0;i<config.tower_max;i++)
		if(config.tower_array[i].id>0)
			process(grid,&config.tower_array[i]);
}



/////bulet

bullet* newBullet(){
	int i;
	for(i=0;i<config.bullet_max;i++)
		if (config.bullet_array[i].id==0){
			config.bullet_array[i].id=getGlobalId();
			return &config.bullet_array[i];
		}
	return 0;
}

void tickMiscBullet(gnode * grid,bullet * b){
	
}

void tickCleanBullet(gnode * grid,bullet * b){
	
}

void tickProcessBullet(gnode * grid,bullet * b){
	
}



void forEachBullet(gnode* grid, void (process)(gnode*g,bullet*b)){
	int i;
	for(i=0;i<config.bullet_max;i++)
		if(config.bullet_array[i].id>0)
			process(grid,&config.bullet_array[i]);
}


//////////////players

void setupPlayer(int id,int isfriend,int base_health){
	config.players[id].id=getGlobalId();
	config.players[id].isfriend=isfriend;
	config.players[id].base_health=base_health;
}
