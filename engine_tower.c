#include "grid.h"
#include "engine.h"
#include "engine_npc.h"
#include "engine_tower.h"
#include "engine_bullet.h"
#include "gridmath.h"


tower* damageTower(tower* t,bullet* b){
//	if(t->type==BASE){
//		config.players[t->owner].base_health-=b->damage;
//		setMask((&config.players[t->owner]),PLAYER_HEALTH);
//	}else{
		t->health-=b->damage;
		setMask(t,TOWER_HEALTH);
//	}
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
	if (t!=0){
		if (t->type==BASE){
			t->health=config.players[t->owner].base_health;
		}else{
			t->health=config.tower_types[t->type].health;
		}
		t->energy=config.tower_types[t->type].energy;
	}
}


tower* spawnTower(gnode * grid,int node_id,int owner,int type){
	tower* t;
	if (node_id<0 || node_id>=config.gridsize*config.gridsize)
		return 0;
	gnode* node=&grid[node_id];
	if((t=newTower())==0)
		perror("newTower spawnTower");
	t->owner=owner;
	t->position=node_id;
	t->type=type;
	t->bit_mask=0;
	t->level=type;//1;
	setMask(t,TOWER_CREATE);
	setTowerBase(t);
	node->tower=t;
	return t;
}


int removeTower(gnode * grid,tower* t){
	gnode * node=&grid[t->position];
	node->tower=0;
	t->health=-1;
	return 0;
}

int tickMiscTower(gnode* grid,tower* t){
	if (t->attack_count<config.tower_types[t->type].attack_speed)
		t->attack_count++;
	t->bit_mask=0;
	return 0;
}

int tickDiedCheckTower(gnode* grid,tower* t){
	t->target=diedCheckNpc(t->target);
	/**/
	return 0;
}



int tickAttackTower(gnode* grid,tower* t){
	if (t->type==BASE)
		return 0;
	if (t->target==0){
		//find near npc
		int x=idtox(t->position);
		int y=idtoy(t->position);
		int i,j,k;
		int yid,xid;
		npc* tmp;
		for (i=0;i<config.tower_types[t->type].distanse;i++)
			for(j=0;j<config.area_size[i];j++)
				if (((xid=x+config.area_array[i][j].x)>=0 && x+config.area_array[i][j].x<config.gridsize) &&
						((yid=y+config.area_array[i][j].y)>=0 && y+config.area_array[i][j].y<config.gridsize))
					for (k=0;k<MAX_GROUPS;k++)
						if (k!=config.players[t->owner].group)
							for(tmp=grid[to2d(xid,yid)].npcs[k];
									tmp!=0;tmp=tmp->next)
								if (canSee(grid,&(vec){x+0.5,y+0.5},&tmp->position)>0)
									if(sqr(x+0.5-(tmp->position.x))+sqr(y+0.5-(tmp->position.y))<=sqr(config.tower_types[t->type].distanse)){
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
				sqr(config.tower_types[t->type].distanse)){
			t->target=0;
			return 0;
		}
//		printf("\t|%d %d\n",t->attack_count,config.tower_types[t->type].attack_speed);
		if (t->attack_count>=config.tower_types[t->type].attack_speed){
				t->attack_count=0;
				bullet* b;//set params of bullet
				if ((b=newBullet())==0){
					perror("newBullet tickAttackBullet");
					return -1;
				}
				vec position={getGridx(t->position),getGridy(t->position)};
				memcpy(&b->position,&position,sizeof(vec));
				memcpy(&b->source,&position,sizeof(vec));
				b->destination.x=t->target->position.x;
				b->destination.y=t->target->position.y;
				b->type=config.tower_types[t->type].bullet_type;
				b->damage=config.tower_types[t->type].damage;
				b->support=config.tower_types[t->type].support;
				b->group=config.players[t->owner].group;
				b->owner=t->id;
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
	if (t->type==BASE)
		return 0;
	if (t->health>0)
		return 0;
	grid[t->position].tower=0;
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


