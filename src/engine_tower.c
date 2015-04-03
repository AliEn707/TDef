#include "grid.h"
#include "engine.h"
#include "engine_npc.h"
#include "engine_tower.h"
#include "engine_bullet.h"
#include "gridmath.h"
#include "types.h"


tower* damageTower(tower* t,bullet* b){
	tower_type *type=0;
	if (t->type==BASE)
		type=&config.players[t->owner].base_type;
	else
		type=typesTowerGet(t->type);
	if (type==0){
		t->id=0;
	}
	if (t->shield>0){
		t->shield-=b->damage;
		if (t->shield<0)
			t->shield=0;
		t->$shield=0;
		setMask(t,TOWER_SHIELD);
		return t;
	}
	
	int damage=b->damage*(0.06f*type->armor/(1+0.06f*type->armor));
	t->health -= damage?:1;
	setMask(t,TOWER_HEALTH);
	
	t->last_attack = b->owner;//
	return t;
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



tower* diedCheckTower(tower* n){
	if (n==0)
		return 0;
	if (n->type!=BASE)
		if (n->health<=0)
			return 0;
	return n;
}


void setTowerBase(tower* t){
	tower_type *type;
	if (t!=0){
		if (t->type==BASE)
			type=&config.players[t->owner].base_type;
		else
			type=typesTowerGet(t->type);
		
		if (type==0){
			t->id=0;
			return;
		}
		t->health=type->health;
		t->shield=type->shield;
		t->energy=type->energy;
	}
}


tower* spawnTower(gnode * grid,int node_id,int owner,int type){
	tower* t;
	tower_type *ttype=0;
//	printDebug("spawn tower %d on %d by %d\n",type,node_id,owner);
	if (node_id<0 || node_id>=config.gridsize*config.gridsize)
		return 0;
	if (grid[node_id].tower!=0)
		return 0;
	gnode* node=&grid[node_id];
	if (type==BASE)
			ttype=&config.players[owner].base_type;
		else
			ttype=typesTowerGet(type);
	if (ttype==0)
		return 0;

	if((t=newTower())==0){
		perror("newTower spawnTower");
		return 0;
	}
	t->owner=owner;
	t->position=node_id;
	t->type=type;
	t->bit_mask=0;
	t->level=type;//1;
	setMask(t,TOWER_CREATE);
	setTowerBase(t);
	node->tower=t;
	config.players[owner].money -= ttype->cost;
	config.players[owner].stat.towers_built++;//created tower: save to stats
	config.players[owner].stat.xp += ttype->cost; //TODO: use something better
	setMask(&config.players[owner], PLAYER_MONEY);
	return t;
}


int removeTower(gnode * grid,tower* t){
//	gnode * node=&grid[t->position];
//	node->tower=0;
	t->health=-1;
	t->last_attack=t->owner;
	setMask(t,TOWER_HEALTH);
	return 0;
}

int tickMiscTower(gnode* grid,tower* t){
	tower_type *type;
	t->bit_mask=0;
	if (t->type==BASE)
		return 0;
	type=typesTowerGet(t->type);
	if (type==0){
		t->id=0;
		return 0;
	}
	if (t->attack_count<type->attack_speed)
		t->attack_count++;
	t->$shield++;
	if (t->$shield>SHIELD_RECOVERY && t->$shield%TPS==0 && t->shield<type->shield){
		t->shield+=type->shield/100;
		setMask(t,TOWER_SHIELD);
	}
	return 0;
}

int tickDiedCheckTower(gnode* grid,tower* t){
	t->target=diedCheckNpc(t->target);
	/**/
	return 0;
}



int tickAttackTower(gnode* grid,tower* t){
	tower_type *type;
	if (t->type==BASE)
		return 0;
	type=typesTowerGet(t->type);
	if (type==0){
		t->id=0;
		return 0;
	}
	if (t->target==0){
		//find near npc
		int x=idtox(t->position);
		int y=idtoy(t->position);
		int i,j,k;
		int yid,xid;
		npc* tmp;
		for (i=0;i<type->distanse;i++)
			for(j=0;j<config.area_size[i];j++)
				if (((xid=x+config.area_array[i][j].x)>=0 && x+config.area_array[i][j].x<config.gridsize) &&
						((yid=y+config.area_array[i][j].y)>=0 && y+config.area_array[i][j].y<config.gridsize))
					for (k=0;k<MAX_GROUPS;k++)
						if (k!=config.players[t->owner].group)
							for(tmp=grid[to2d(xid,yid)].npcs[k];
									tmp!=0;tmp=tmp->next)
								if (canSee(grid,&(vec){x+0.5,y+0.5},&tmp->position)>0)
									if(sqr(x+0.5-(tmp->position.x))+sqr(y+0.5-(tmp->position.y))<=sqr(type->distanse)){
										t->target=tmp;
										setMask(t,TOWER_TARGET);
										if (rand()%100<60)
											return 0;
									}
	}else{
		if (t->target->id==0)
			return 0;
		if(rand()%100<8){
			t->target=0;
			return 0;
		}
		if (sqr(t->target->position.x-getGridx(t->position))+
				sqr(t->target->position.y-getGridy(t->position))>
				sqr(type->distanse)){
			t->target=0;
			return 0;
		}
//		printDebug("\t|%d %d\n",t->attack_count,type->attack_speed);
		if (t->attack_count>=type->attack_speed){
				t->attack_count=0;
				bullet* b;//set params of bullet
				if ((b=newBullet())==0){
					perror("newBullet tickAttackBullet");
					return -1;
				}
				vec position={getGridx(t->position),getGridy(t->position)};
				// getTargetPos(vec *pos, vec *dir, float v, vec * $pos, float $v){
//				vec* target=getTargetPos(&t->target->position,
//									&t->target->direction,
//									config.npc_types[t->target->type].move_speed,
//									&position,
//									config.bullet_types[type->bullet_type].speed
//									);
				memcpy(&b->position,&position,sizeof(vec));
				memcpy(&b->source,&position,sizeof(vec));
				//memcpy(&b->destination,target,sizeof(vec));
//				printDebug("aaaa = %g|%g {%g|%g}\n",target->x,target->y,t->target->position.x,t->target->position.y);
				b->ntarget=t->target;
				b->destination.x=t->target->position.x;
				b->destination.y=t->target->position.y;
				b->max_dist=type->distanse;
				b->type=type->bullet_type;
				b->damage=type->damage;
				b->support=type->support;
				b->group=config.players[t->owner].group;
				b->owner=t->owner;
				setMask(b,BULLET_CREATE);
//				b->target=NPC;
				getDir(&b->position,&b->destination,&b->direction);
//				memcpy(&b->effects,&config.npc_types[n->type].effects,sizeof(effects));
			}
	}
	/**/
	return 0;
}

int tickCleanTower(gnode* grid,tower* t){
	tower_type *type=0;
	if (t->health>0)
		return 0;
	if (t->type==BASE){
		printDebug("player %d lose\n",t->owner);
		config.players[t->owner].base=0;
		setMask(&config.players[t->owner], PLAYER_FAIL);
//		return 0;
	}
	if (t->type==BASE)
		type=&config.players[t->owner].base_type;
	else
		type=typesTowerGet(t->type);
	if (type==0){
		t->id=0;
		return 0;
	}
	grid[t->position].tower=0;
	if (t->last_attack>0){
		//get cash, if enemy -> receve, if player's own -> 0.5 of cost
		config.players[t->last_attack].money += t->last_attack==t->owner?type->cost/2:type->receive;
		config.players[t->last_attack].stat.towers_destroyed++;//attacking player destroyed tower
		config.players[t->last_attack].stat.xp += t->last_attack==t->owner?type->cost/2:type->receive; //TODO: use something better
		setMask(&config.players[t->last_attack], PLAYER_MONEY);
	}
	config.players[t->owner].stat.towers_lost++;//tower's owner lost tower
	memset(t,0,sizeof(tower));
	/**/
	return 0;
}



int forEachTower(gnode* grid, int (process)(gnode*g,tower*t)){//add function
	int i;
	for(i=0;i<config.tower_max;i++)
		if(config.tower_array[i].id>0)
			if(process(grid,&config.tower_array[i])<0)
				return -1;
	return 0;
}


