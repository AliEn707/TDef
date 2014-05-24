#include "grid.h"
#include "engine.h"


int timePassed(){
	//config.time  struct timeval
	struct timeval end;
	gettimeofday(&end, NULL);
	int out=(end.tv_usec - config.time.tv_usec);
	if(config.time.tv_usec!=0)
		if (out>1000000/TPS)
			perror("time to tick");
	memcpy(&config.time,&end,sizeof(end));
	return out;
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

tower* newTower(){
	int i;
	for(i=0;i<config.tower_max;i++)
		if (config.tower_array[i].id==0){
			config.tower_array[i].id=getGlobalId();
			return &config.tower_array[i];
		}	
	return 0;
}

bullet* newBullet(){
	int i;
	for(i=0;i<config.bullet_max;i++)
		if (config.bullet_array[i].id==0){
			config.bullet_array[i].id=getGlobalId();
			return &config.bullet_array[i];
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

tower* findNearTower(gnode* grid,npc* n){
	/**/
	int i,j,k;
	int x=(int)n->position.x;
	int y=(int)n->position.y;
	int yid,xid;
	for(i=0;i<config.npc_types[n->type].see_distanse;i++)
		for(j=0;j<config.area_size[i];j++)
			if (((xid=x+config.area_array[i][j].x)>=0 && x+config.area_array[i][j].x<config.gridsize) &&
					((yid=y+config.area_array[i][j].y)>=0 && y+config.area_array[i][j].y<config.gridsize))
				if (grid[to2d(xid,yid)].tower!=0)
					if(config.players[grid[to2d(xid,yid)].tower->owner].isfriend!=n->isfriend){
						n->ttarget=grid[to2d(xid,yid)].tower;
						if(rand()%100<40)
							return n->ttarget;
					}
			
//	config.area_array
//	config.tower_types[i].see_distanse
	if (n->ttarget==0)
		return 0;
	return n->ttarget;
}
void tickTargetNpc(gnode* grid,npc* n){
	if ((n->ttarget=findNearTower(grid,n))!=0)
		return;
	int id;
	if((id=findEnemyBase((int)n->isfriend))<0){
		perror("findEnemyBase tickTargetNpc");
		return;
	}
	if ((n->ttarget=grid[id].tower)==0)
		perror("ttarget tickTargetNpc");
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

void forEachNpc(gnode* grid, void (process)(gnode*g,npc*n)){//add function
	int i;
	for(i=0;i<config.npc_max;i++)
		if(config.npc_array[i].id>0)
			process(grid,&config.npc_array[i]);
}

//////////////towers

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

int removeTower(gnode * grid,tower* t){
	gnode * node=&grid[t->position];
	node->tower=0;
	memset(t,0,sizeof(tower));
	return 0;
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

//////////////players

void setupPlayer(int id,int isfriend,int base_health){
	config.players[id].id=getGlobalId();
	config.players[id].isfriend=isfriend;
	config.players[id].base_health=base_health;
}
