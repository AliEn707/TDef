#include "grid.h"
#include "engine.h"


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
//	printf("%d %d %d\n",sizeof(tower)*config.tower_max,sizeof(npc)*config.npc_max,sizeof(bullet)*config.bullet_max);
}

void realizeArrays(){
	free(config.tower_array);
	free(config.npc_array);
	free(config.bullet_array);
}


npc* newNpc(){
	int i;
	for(i=0;i<config.npc_max;i++)
		if (config.npc_array[i].id==0){
			config.npc_array[i].id=config.global_id++;
			return &config.npc_array[i];
		}	
	return 0;
}

tower* newTower(){
	int i;
	for(i=0;i<config.tower_max;i++)
		if (config.tower_array[i].id==0){
			config.tower_array[i].id=config.global_id++;
			return &config.tower_array[i];
		}	
	return 0;
}

bullet* newBullet(){
	int i;
	for(i=0;i<config.bullet_max;i++)
		if (config.bullet_array[i].id==0){
			config.bullet_array[i].id=config.global_id++;
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
	memcpy(&n->destination,&n->position,sizeof(vec));
	n->type=type;
	setNpcBase(n);
	if (addNpc(&grid[node_id],n)==0)
		perror("addMpc spawnNpc");
	return n;
}

void tickMoveNpc(gnode* grid,npc* n){
	if (n->status==IN_MOVE){
		
		//check path from position
		if (eqInD(n->position.x,n->destination.x,config.npc_types[n->type].move_speed) &&
					eqInD(n->position.y,n->destination.y,config.npc_types[n->type].move_speed)){
			int node_id=getGridId(n->position);
			node_id=grid[node_id].next;
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

void setTowerBase(tower* t){
	
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
