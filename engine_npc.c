#include "grid.h"
#include "engine.h"
#include "engine_npc.h"
#include "engine_tower.h"
#include "engine_bullet.h"
#include "gridmath.h"

npc* newNpc(){
	int i;
	for(i=0;i<config.npc_max;i++)
		if (config.npc_array[i].id==0){
			config.npc_array[i].id=getGlobalId();
			config.npc_num++;
			return &config.npc_array[i];
		}	
	return 0;
}


npc* addNpc(gnode* node,npc* n){
	npc **root=&node->npcs[(int)n->isfriend];
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
	gnode * node=&grid[(int)getGridId(n->position)];
	npc **root=&node->npcs[(int)n->isfriend];
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
		config.npc_num--;
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
	n->bit_mask=0;
	setMask(n,NPC_CREATE);
	memcpy(&n->destination,&n->position,sizeof(vec));
	n->type=type;
	setNpcBase(n);
	if (addNpc(&grid[node_id],n)==0)
		perror("addMpc spawnNpc");
	return n;
}



tower* findNearTower(gnode* grid,npc* n,int range){
	/**/
	int i,j;
	int x=(int)n->position.x;
	int y=(int)n->position.y;
	int yid,xid;
//	printf("%d\n",n->id);
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

npc* findNearNpc(gnode* grid,npc* n,int range){
	/**/
	int i,j,k;
	int x=(int)n->position.x;
	int y=(int)n->position.y;
	int yid,xid;
	npc* tmp;
//	printf("%d\n",n->id);
	for(i=0;i<range;i++){
//		printf("!! %d\n",range);
		for(j=0;j<config.area_size[i];j++)
			if (((xid=x+config.area_array[i][j].x)>=0 && x+config.area_array[i][j].x<config.gridsize) &&
					((yid=y+config.area_array[i][j].y)>=0 && y+config.area_array[i][j].y<config.gridsize))
				for (k=0;k<MAX_PLAYERS;k++)
						if (k!=n->isfriend)
							for(tmp=grid[to2d(xid,yid)].npcs[k];
									tmp!=0;tmp=tmp->next)
								if (canSee(grid,&n->position,&tmp->position)>0){
									n->ntarget=tmp;
									if (rand()%100<60)
										return n->ntarget;
								}
		if(n->ntarget!=0)
			return n->ntarget;
	}
//	config.area_array
//	config.tower_types[i].see_distanse
	return n->ntarget;
}

npc* diedCheckNpc(npc* n){
	if (n==0)
		return 0;
	if (n->health<=0)
		return 0;
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


int tickTargetNpc(gnode* grid,npc* n){
		if(n->ttarget==0 && n->ntarget==0){
			n->path_count=NPC_PATH;
			n->status=IN_MOVE;
			if (findNearTower(grid,n,config.npc_types[n->type].see_distanse)!=0)
				goto out;
			
			//if near no Towers
			if (findNearNpc(grid,n,config.npc_types[n->type].see_distanse)!=0)
				goto out;
			
			int id;
			if((id=findEnemyBase((int)n->isfriend))<0)
				return 0;  
			
			if ((n->ttarget=grid[id].tower)==0)
				perror("ttarget tickTargetNpc");
			if (n->ttarget==0 && n->ntarget==0){
				n->status=IN_IDLE;
				return 0;
			}
out:
			memcpy(&n->destination,&n->position,sizeof(vec));
		}
	return 0;
}


int tickAttackNpc(gnode* grid,npc* n){
	if (n->status==IN_ATTACK){
		//if target !=0
		//-attacking
		//else set IN_MOVE
		//???????
		
//		printf("\t %d %d %d\n",n->id,n->attack_count,config.npc_types[n->type].attack_speed);
		if (n->ntarget!=0)
			if (n->attack_count>=config.npc_types[n->type].attack_speed){
				n->attack_count=0;
				bullet* b;//set params of bullet
				if ((b=newBullet())==0){
					perror("newBullet tickAttackBullet");
					return -1;
				}
				memcpy(&b->position,&n->position,sizeof(vec));
				memcpy(&b->source,&n->position,sizeof(vec));
				memcpy(&b->destination,&n->ntarget->position,sizeof(vec));
				b->type=config.npc_types[n->type].bullet_type;
				b->damage=config.npc_types[n->type].damage;
				b->support=config.npc_types[n->type].support;
				b->isfriend=n->isfriend;
				b->owner=n->id;
				setMask(b,BULLET_CREATE);
//				b->target=TOWER;
				getDir(&b->position,&b->destination,&b->direction);
//				memcpy(&b->effects,&config.npc_types[n->type].effects,sizeof(effects));
				return 0;
				
			}
		if (n->ttarget!=0)
			if (n->attack_count>=config.npc_types[n->type].attack_speed){
				n->attack_count=0;
				bullet* b;//set params of bullet
				if ((b=newBullet())==0){
					perror("newBullet tickAttackBullet");
					return -1;
				}
				memcpy(&b->position,&n->position,sizeof(vec));
				memcpy(&b->source,&n->position,sizeof(vec));
				b->destination.x=getGridx(n->ttarget->position);
				b->destination.y=getGridy(n->ttarget->position);
				b->type=config.npc_types[n->type].bullet_type;
				b->damage=config.npc_types[n->type].damage;
				b->support=config.npc_types[n->type].support;
				b->isfriend=n->isfriend;
				b->owner=n->id;
				setMask(b,BULLET_CREATE);
//				b->target=TOWER;
				getDir(&b->position,&b->destination,&b->direction);
//				memcpy(&b->effects,&config.npc_types[n->type].effects,sizeof(effects));
				return 0;
			}
	}else{
		//search target in attack distanse
		//if finded set IN_ATTACK
		npc* ntmp=n->ntarget;
		n->ntarget=0;
		if (findNearNpc(grid,n,config.npc_types[n->type].attack_distanse)!=0){
			n->status=IN_ATTACK;
			return 0;
		}
		n->ntarget=ntmp;
		tower* ttmp=n->ttarget;
		n->ttarget=0;
		if (findNearTower(grid,n,config.npc_types[n->type].attack_distanse)!=0){
			n->status=IN_ATTACK;
			n->path_count=NPC_PATH;
			return 0;
		}
		n->ttarget=ttmp;
	}
	return 0;
}
  
int tickDiedCheckNpc(gnode* grid,npc* n){
	//need to correct
	n->ntarget=diedCheckNpc(n->ntarget);
	n->ttarget=diedCheckTower(n->ttarget);
	return 0;
}




int tickCleanNpc(gnode* grid,npc* n){
	if (n->health>0)
		return 0;
	delNpc(grid,n);
	memset(n,0,sizeof(npc));
	//foeachNpc
	return 0;
}

int tickMoveNpc(gnode* grid,npc* n){
	if (n->status==IN_MOVE){
		if (n->ttarget!=0 || n->ntarget!=0){
			if (n->ttarget!=0){
			//check path from position
				if ((eqInD(n->position.x,n->destination.x,config.npc_types[n->type].move_speed) &&
							eqInD(n->position.y,n->destination.y,config.npc_types[n->type].move_speed))||
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
					getDir(&n->position,&n->destination,&n->direction);
				}
			}else 
				if (n->ntarget!=0)
					//check path from position
					if ((eqInD(n->position.x,n->destination.x,config.npc_types[n->type].move_speed) &&
								eqInD(n->position.y,n->destination.y,config.npc_types[n->type].move_speed))||
						glength(&n->position,&n->destination)<0.05){
						if (n->path_count>=NPC_PATH){
							memset(n->path,-1,sizeof(int)*NPC_PATH);
							if(aSearch(grid,
									grid+getGridId(n->ntarget->position),
									grid+getGridId(n->position),
									n->path)<0)
								perror("aSearch tickMoveNpc");
							n->path_count=0;
							}
						
						int node_id;
						node_id=n->path[n->path_count++];
						n->destination.x=getGridx(node_id);
						n->destination.y=getGridy(node_id);
						getDir(&n->position,&n->destination,&n->direction);
					}
			
			//vec dir={0,0};
			//getDir(&n->position,&n->destination,&dir);
			
			vec pos={n->position.x+n->direction.x*config.npc_types[n->type].move_speed,
					n->position.y+n->direction.y*config.npc_types[n->type].move_speed};
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
			setMask(n,NPC_POSITION);
		}
	}
	return 0;
}

int tickMiscNpc(gnode* grid,npc* n){
	if (n->attack_count<config.npc_types[n->type].attack_speed)
		n->attack_count++;
	n->bit_mask=0;
	return 0;
}


int forEachNpc(gnode* grid, int (process)(gnode*g,npc*n)){//add function
	int i;
	for(i=0;i<config.npc_max;i++)
		if(config.npc_array[i].id>0)
			if (process(grid,&config.npc_array[i])!=0)
				return -1;
	return 0;
}
