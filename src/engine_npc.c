#include "grid.h"
#include "engine.h"
#include "engine_npc.h"
#include "engine_tower.h"
#include "engine_bullet.h"
#include "gridmath.h"
#include "types.h"

npc* damageNpc(npc* n,bullet* b){
	npc_type * type=0;
	if (n->type==HERO)
		type=&config.players[n->owner].hero_type;
	else
		type=typesNpcGet(n->type);
	
	if (type==0){
		n->id=0;
		return n;
	}
	if (n->shield>0){
		n->shield-=b->damage;
		if (n->shield<0)
			n->shield=0;
		n->$shield=0;
		setMask(n,NPC_SHIELD);
		return n;
	}
	int damage=b->damage*(1-0.06f*type->armor/(1+0.06f*type->armor));
//	printDebug("(%d * %g)=%d damage\n",b->damage,(1-0.06f*type->armor/(1+0.06f*type->armor)),damage);
	n->health -= damage?:1;
	n->last_attack = b->owner;//save last attacking player to 
	setMask(n,NPC_HEALTH);
	return n;
}

npc* newNpc(){
	if (config.npc_num==config.npc_max)
		return 0;
	if ((config.npc_array[config.npc_num]=malloc(sizeof(npc)))==0)
		return 0;
	memset(config.npc_array[config.npc_num],0,sizeof(npc));
	config.npc_array[config.npc_num]->id=getGlobalId();
	config.npc_num++;
	return config.npc_array[config.npc_num-1];
}


npc* addNpc(gnode* node,npc* n){
	npc **root=&node->npcs[config.players[n->owner].group];
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
	npc **root=&node->npcs[config.players[n->owner].group];
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
	int i;
	for(i=0;i<config.npc_num && config.npc_array[i]!=tmp;i++);
	if (i==config.npc_num)
		return -1;
	free(config.npc_array[i]);
	config.npc_num--;
	if (config.npc_num!=i){
		config.npc_array[i]=config.npc_array[config.npc_num];
	}
	config.npc_array[config.npc_num]=0;
	return -1;
}

void setNpcBase(npc* n){
	npc_type * type=0;
	if (n->type==HERO)
		type=&config.players[n->owner].hero_type;
	else
		type=typesNpcGet(n->type);
	
	if (type==0){
		n->health=-1;
		return;
	}
	n->health=type->health;
	n->shield=type->shield;
	n->energy=type->energy;
//	printDebug("%d seted %d %d \n",n->id,type->health,type->shield);
	//may be more
}


npc* spawnNpc(gnode* grid,int node_id,int owner,int type){
	npc* n;
	npc_type* ntype=0;
	if (type==HERO)
		ntype=&config.players[owner].hero_type;
	else
		ntype=typesNpcGet(type);
	if (ntype==0)
		return 0;
	if((n=newNpc())==0){
		perror("newNpc spawnNpc");
		return 0;
	}
	n->owner=owner;
	n->status=IN_MOVE;
	n->position.x=getGridx(node_id);
	n->position.y=getGridy(node_id);
	n->path_count=NPC_PATH;
	n->level=type;	
//	n->level=0;	
	n->bit_mask=0;
	setMask(n,NPC_CREATE);
	memcpy(&n->destination,&n->position,sizeof(vec));
	n->type=type;
	setNpcBase(n);
	if (addNpc(&grid[node_id],n)==0)
		perror("addMpc spawnNpc");
	config.players[owner].money -= ntype->cost;
	config.players[owner].stat.npcs_spawned++;//spawned npc: save to stats
	config.players[owner].stat.xp += ntype->cost; //TODO: use something better
	setMask(&config.players[owner], PLAYER_MONEY);
	return n;
}



tower* findNearTower(gnode* grid,npc* n,int range){
	/**/
	int i,j;
	int x=(int)n->position.x;
	int y=(int)n->position.y;
	int yid,xid;
	npc_type * type=0;
	if (n->type==HERO)
		type=&config.players[n->owner].hero_type;
	else
		type=typesNpcGet(n->type);
	if (type==0){
		n->health=-1;
		return 0;
	}
//	printDebug("%d\n",n->id);
	for(i=0;i<range;i++){
		for(j=0;j<config.area_size[i];j++)
			if (((xid=x+config.area_array[i][j].x)>=0 && x+config.area_array[i][j].x<config.gridsize) &&
					((yid=y+config.area_array[i][j].y)>=0 && y+config.area_array[i][j].y<config.gridsize))
				if (grid[to2d(xid,yid)].tower!=0)
					if (type->attack_tower!=0 || grid[to2d(xid,yid)].tower->type==BASE)
						if(config.players[grid[to2d(xid,yid)].tower->owner].group!=config.players[n->owner].group)
							if (canSee(grid,&(vec){n->position.x,n->position.y},&(vec){xid+0.5,yid+0.5})>0 && rand()%100<80) //can see check, in 70%
	//							if(canWalkThrough(grid,&(vec){n->position.x,n->position.y},&(vec){xid+0.5,yid+0.5})>0){//try this || rand()%100<30){//can walk check or rand<30%
								if(sqr(n->position.x-(xid+0.5))+sqr(n->position.y-(yid+0.5))<=sqr(range)){//try this || rand()%100<30){//can walk check or rand<30%
									n->ttarget=grid[to2d(xid,yid)].tower;
	//								printDebug("? %d\n",to2d(xid,yid));
									if(rand()%100<30)
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
//	printDebug("%d\n",n->id);
	for(i=0;i<range;i++){
//		printDebug("!! %d\n",range);
		for(j=0;j<config.area_size[i];j++)
			if (((xid=x+config.area_array[i][j].x)>=0 && x+config.area_array[i][j].x<config.gridsize) &&
					((yid=y+config.area_array[i][j].y)>=0 && y+config.area_array[i][j].y<config.gridsize))
				for (k=0;k<MAX_GROUPS;k++)
						if (k!=config.players[n->owner].group)
							for(tmp=grid[to2d(xid,yid)].npcs[k];
									tmp!=0;tmp=tmp->next)
								if (canWalkThrough(grid,&n->position,&tmp->position)>0)
									if(sqr(n->position.x-(tmp->position.x))+sqr(n->position.y-(tmp->position.y))<=sqr(range)){
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


int findEnemyBase(int group){
	#define t config.tower_array
	int i;
	int id=-1;
	for(i=0;i<config.tower_max;i++)
		if (t[i].id>0)
			if (t[i].type==BASE)
				if (config.players[t[i].owner].group!=group){//add friend check
					id=t[i].position;
					if (rand()%100<40)
						return id;
				}
	return id;
	#undef t
}


int tickTargetNpc(gnode* grid,npc* n){
	npc_type * type=0;
	if (n->type==HERO)//Hero not search for
		return 0;
//		type=&config.players[n->owner].hero_type;
	else
		type=typesNpcGet(n->type);
	if (type==0){
		n->health=-1;
		return 0;
	}
	if (config.players[n->owner].target_changed){
		n->ttarget=0;
		n->ntarget=0;
		n->finded_base=-1;
	}
	if(n->ttarget==0 && n->ntarget==0){
		int id=-1;
		n->path_count=NPC_PATH;
		n->status=IN_MOVE;
		if (findNearTower(grid,n,type->see_distanse)!=0)
			goto out;
		
		//if near no Towers
		if (findNearNpc(grid,n,type->see_distanse)!=0)
			goto out;
		//if player set target
		if (config.players[n->owner].target>0){ //1 is o index
			if (config.players[config.players[n->owner].target-1].base!=0){
				id=config.bases[config.players[config.players[n->owner].target-1].base_id].position;
//				printf("set %d\n",id);
			}else{
				config.players[n->owner].target=0; //set to random
				setMask(&config.players[n->owner],PLAYER_TARGET);
//				printf("set to rand\n");
			}
			n->finded_base=-1;
		} else
			if (config.players[n->owner].target<0){ //set follow hero
				if (config.players[n->owner].hero!=0){
					n->ntarget=config.players[n->owner].hero;
				}
				goto out;
			}

		//try to go to previous base
		if (n->finded_base>=0){
			if (config.players[n->finded_base].base==0){
//				printDebug("not found base %d\n",n->finded_base);
				n->finded_base=-1;
			}else{
				id=config.bases[config.players[n->finded_base].base_id].position;
//				printDebug("set id = %d\n",id);
			}
		}
		
		if (id<0)
			if((id=findEnemyBase(config.players[n->owner].group))<0)
				return 0;  
		
//		printDebug("found %d \n",id);
		if ((n->ttarget=grid[id].tower)!=0){
			n->finded_base=grid[id].tower->owner;
//			printDebug("set base founded = %d\n",n->finded_base);
		}//else{
//			perror("ttarget tickTargetNpc"); //no base found
//			printDebug("on %d = %d\n",id,grid[id].tower);
//		}
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
	npc_type * type=0;
	if (n->type==HERO)
		type=&config.players[n->owner].hero_type;
	else
		type=typesNpcGet(n->type);
	if (type==0){
		n->health=-1;
		return 0;
	}
	if (n->status==IN_ATTACK || (rand()%100<20 && n->attack_count<type->attack_speed)){
		//if target !=0
		//-attacking
		//else set IN_MOVE
		//???????
//		if (rand()%100<8){
//			n->ntarget=0;
//			n->ttarget=0;
//			return 0;
//		}
//		printDebug("\t %d %d %d\n",n->id,n->attack_count,type->attack_speed);
		if (n->ntarget!=0){
			if (n->ntarget==&config.players[n->owner].$npc$){
				n->status=IN_MOVE;
				return 0;
			}
			if (sqr(n->ntarget->position.x-n->position.x)+
					sqr(n->ntarget->position.y-n->position.y)>
					sqr(type->see_distanse)){
				n->ntarget=0;
				return 0;
			}
			if (sqr(n->ntarget->position.x-n->position.x)+
					sqr(n->ntarget->position.y-n->position.y)>
					sqr(type->attack_distanse)){
				n->status=IN_MOVE;
				return 0;
			}
			if (n->attack_count>=type->attack_speed){
				n->attack_count=0;
				bullet* b;//set params of bullet
				if ((b=newBullet())==0){
					perror("newBullet tickAttackBullet");
					return -1;
				}
				memcpy(&b->position,&n->position,sizeof(vec));
				memcpy(&b->source,&n->position,sizeof(vec));
				memcpy(&b->destination,&n->ntarget->position,sizeof(vec));
				b->type=type->bullet_type;
				b->damage=type->damage;
				b->support=type->support;
				b->group=config.players[n->owner].group;
				b->owner=n->owner;
				setMask(b,BULLET_CREATE);
	//			b->target=TOWER;
				getDir(&b->position,&b->destination,&b->direction);
	//			memcpy(&b->effects,&type->effects,sizeof(effects));
				return 0;
			}
		}
		if (n->ttarget!=0){
			if (sqr(n->position.x-getGridx(n->ttarget->position))+
					sqr(n->position.y-getGridy(n->ttarget->position))>
					sqr(type->see_distanse)){
				n->ttarget=0;
				return 0;
			}
			if (sqr(n->position.x-getGridx(n->ttarget->position))+
					sqr(n->position.y-getGridy(n->ttarget->position))>
					sqr(type->attack_distanse)){
				n->status=IN_MOVE;
				return 0;
			}
			if (n->attack_count>=type->attack_speed){
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
				b->type=type->bullet_type;
				b->damage=type->damage;
				b->support=type->support;
				b->group=config.players[n->owner].group;
				b->owner=n->owner;
				setMask(b,BULLET_CREATE);
//				b->target=TOWER;
				getDir(&b->position,&b->destination,&b->direction);
//				memcpy(&b->effects,&type->effects,sizeof(effects));
				return 0;
			}
		}
	}else{
		//search target in attack distanse
		//if finded set IN_ATTACK
		npc* ntmp=n->ntarget;
		n->ntarget=0;
		if (findNearNpc(grid,n,(rand()%100<60)?type->see_distanse:type->attack_distanse)!=0){
			n->status=IN_ATTACK;
			n->path_count=NPC_PATH;
			return 0;
		}
		n->ntarget=ntmp;
		tower* ttmp=n->ttarget;
		n->ttarget=0;
		if (findNearTower(grid,n,(rand()%100<60)?type->see_distanse:type->attack_distanse)!=0){
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
	npc_type *type=0;
	if (n->health>0)
		return 0;
	if (n->type==HERO)
		type=&config.players[n->owner].hero_type;
	else
		type=typesNpcGet(n->type);
	if (type==0){
		n->health=-1;
		return 0;
	}
	if (n->last_attack>0){
		config.players[n->last_attack].money += type->receive;
		config.players[n->last_attack].stat.npcs_killed++;//n->last_attack killed one more npc
		config.players[n->last_attack].stat.xp += type->receive; //TODO: use something better
		setMask(&config.players[n->last_attack], PLAYER_MONEY);
	}
	config.players[n->owner].stat.npcs_lost++;//n->owner lost one more npc
	if (n->type==HERO){
		config.players[n->owner].hero=0;
		setMask(&config.players[n->owner],PLAYER_HERO);
		printDebug("hero of %d died\n",n->owner);
	}
	delNpc(grid,n);
	//foeachNpc
	return 0;
}

int tickMoveNpc(gnode* grid,npc* n){
	int i;
	npc_type * type=0;
	if (n->type==HERO)
		type=&config.players[n->owner].hero_type;
	else
		type=typesNpcGet(n->type);
	if (type==0){
		n->id=0;
		return 0;
	}
	//TODO: may be not need	
	if (n->destination.x<0 || n->destination.y<0)
			memcpy(&n->destination,&n->position,sizeof(n->position));
		
	if (n->status==IN_MOVE){
		if (n->ttarget!=0 || n->ntarget!=0){
			if (n->ntarget!=0)
				if (eqInD(n->position.x,n->ntarget->position.x,type->move_speed<1?1:type->move_speed) &&
						eqInD(n->position.y,n->ntarget->position.y,type->move_speed<1?1:type->move_speed)){
					n->ntarget=0;
					n->path_count=NPC_PATH;
					printDebug("\ndest reached\n");
					return 0;
				}
/*			//why it happens??	
			if (n->path_count>0 && n->path_count<NPC_PATH-1)
				if (n->path[n->path_count-1].node==n->path[n->path_count+1].node){
					n->path_count=NPC_PATH;
					memcpy(&n->destination,&n->position,sizeof(vec));
				}
*/
			//check path from position
			if ((eqInD(n->position.x,n->destination.x,type->move_speed) &&
						eqInD(n->position.y,n->destination.y,type->move_speed))||
				glength(&n->position,&n->destination)<0.05 ||
				n->path[n->path_count].node==-1){
				if (n->path_count>=NPC_PATH || 
						n->path[n->path_count].node==-1 || 
						(grid[n->path[n->path_count].node].tower!=0 && n->path[n->path_count].tower<0)){
					for(i=0;i<NPC_PATH;i++)
							n->path[i].node=-1;
					if(aSearch(grid,
							n->ttarget!=0?
								(grid+n->ttarget->position):
								(grid+getGridId(n->ntarget->position)),
							grid+getGridId(n->position),
							n->path)<0);
//						perror("aSearch tickMoveNpc");
					n->path_count=0;
//					printf("wrong\n");
					}
				
				int node_id;
				node_id=n->path[n->path_count++].node;
				if (node_id<0){
					//we don't have path 
//					memcpy(&n->destination,&n->position,sizeof(n->position));
					memset(&n->direction,0,sizeof(n->direction));
				}else{
					//path ok
					n->destination.x=getGridx(node_id);
					n->destination.y=getGridy(node_id);
					getDir(&n->position,&n->destination,&n->direction);
				}
				setMask(n,NPC_POSITION);
			}
			
			vec pos={n->position.x+n->direction.x*type->move_speed,
					n->position.y+n->direction.y*type->move_speed};
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
//			setMask(n,NPC_POSITION);
		}
	}
	return 0;
}

int tickMiscNpc(gnode* grid,npc* n){
	npc_type * type=0;
	n->bit_mask=0;
	if (n->type==HERO)
		type=&config.players[n->owner].hero_type;
	else
		type=typesNpcGet(n->type);
	if (type==0){
		n->health=-1;
		return 0;
	}
	if (n->attack_count<type->attack_speed)
		n->attack_count++;
	n->$shield++;
	if (n->$shield>SHIELD_RECOVERY && n->$shield%TPS==0 && n->shield<type->shield){
		short add=type->shield/100;
		n->shield+=add?:1;
		setMask(n,NPC_SHIELD);
	}
	return 0;
}


int forEachNpc(gnode* grid, int (process)(gnode*g,npc*n)){//add function
	int i;
	for(i=0;i<config.npc_num;i++)
		if (process(grid,config.npc_array[i])!=0)
			return -1;
	return 0;
}


int setHeroTargetByNode(gnode * grid,npc* n, int node){
	npc * fake=&config.players[n->owner].$npc$;
	if (grid[node].walkable<=0)
		return -1;
	fake->position.x=getGridx(node);
	fake->position.y=getGridy(node);
	fake->health=1;
	n->ntarget=fake;
	n->path_count=0;
	memset(n->path,-1,sizeof(int)*NPC_PATH);
	n->status=IN_MOVE;
	return 0;
}
